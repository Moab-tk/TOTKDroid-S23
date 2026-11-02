// Android: which Vulkan driver the backend uses. LARECOMP_VK_DRIVER=<path of a driver library in the app's internal
// storage> (the activity sets it from the user's choice, e.g. Turnip): loaded through adrenotools, in a linker
// namespace of its own (its hooks are in nativeLibraryDir, LARECOMP_NATIVE_LIB_DIR). Otherwise, or if that fails, the
// system driver. volk then takes every Vulkan function from it (device.c: volkLoadInstance / volkLoadDevice).
#include <adrenotools/driver.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../../settings.h"
#include "vk.h"

// (S23) Settings > Video > Tests: the GPU doesn't rest between frames (adrenotools_set_turbo, Eden's "force max clock").
// The state is the kernel's, for the whole phone: it outlives the app. So it follows the app being in front (onPause /
// onResume), goes off on the way out and in the crash handler, and every start clears what a killed run left. The same
// call as adrenotools', for the kernel's answer; open / ioctl / close only: safe in a signal handler.
static int turbo_ioctl(bool on) {
    struct { unsigned type; void *value; size_t sizebytes; } prop;
    unsigned enable = on ? 0 : 1;
    prop.type = 0xE, prop.value = &enable, prop.sizebytes = sizeof enable;  // (KGSL_PROP_PWRCTRL)
    int fd = open("/dev/kgsl-3d0", O_RDWR | O_CLOEXEC);
    if (fd < 0) return errno;
    int r = ioctl(fd, _IOW(0x09, 0x32, prop), &prop) < 0 ? errno : 0;  // (IOCTL_KGSL_SETPROPERTY)
    close(fd);
    return r;
}

static atomic_bool g_turbo;
void vk_gpu_turbo_off_now(void) {
    if (atomic_exchange(&g_turbo, false)) turbo_ioctl(false);
}

void vk_gpu_turbo(bool front) {
    bool on = front && g_settings.gpu_turbo;
    if (on == atomic_exchange(&g_turbo, on)) return;
    int err = turbo_ioctl(on);
    LOG("[vk] GPU kept awake: %s (%s)\n", on ? "on" : "off", err ? strerror(err) : "ok");
}

void vk_gpu_turbo_start(void) {
    atomic_store(&g_turbo, false);
    turbo_ioctl(false);
    atexit(vk_gpu_turbo_off_now);
    vk_gpu_turbo(true);
}

// Does this driver offer a Vulkan GPU at all? (Turnip v26.0.0 R8, built for the A7xx: none on an Adreno 830 - the app
// stopped at "no Vulkan device" on every start, its settings out of reach.) 1.1 and up: below 1.3 device.c checks the
// extensions it needs instead (COMPAT_EXT).
static bool driver_has_gpu(PFN_vkGetInstanceProcAddr gipa) {
    PFN_vkCreateInstance create = (PFN_vkCreateInstance)gipa(NULL, "vkCreateInstance");
    if (!create) return false;
    VkApplicationInfo app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3};
    VkInstanceCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app};
    VkInstance inst;
    if (create(&ci, NULL, &inst) != VK_SUCCESS) return false;
    PFN_vkEnumeratePhysicalDevices enumerate = (PFN_vkEnumeratePhysicalDevices)gipa(inst, "vkEnumeratePhysicalDevices");
    PFN_vkGetPhysicalDeviceProperties props = (PFN_vkGetPhysicalDeviceProperties)gipa(inst, "vkGetPhysicalDeviceProperties");
    PFN_vkDestroyInstance destroy = (PFN_vkDestroyInstance)gipa(inst, "vkDestroyInstance");
    bool ok = false;
    uint32_t n = 0;
    if (enumerate && props && enumerate(inst, &n, NULL) == VK_SUCCESS && n) {
        VkPhysicalDevice pd[8];
        if (n > 8) n = 8;
        if (enumerate(inst, &n, pd) >= 0)
            for (uint32_t i = 0; i < n && !ok; i++) {
                VkPhysicalDeviceProperties p;
                props(pd[i], &p);
                ok = p.apiVersion >= VK_API_VERSION_1_1;
            }
    }
    if (destroy) destroy(inst, NULL);
    return ok;
}

// The chosen driver the system's stood in for (its package's name; "" none): the start screen and the driver errors
// say so - before, a Turnip build for another Adreno generation fell back silently, and the "driver too old" error
// read as if Turnip lacked the extensions
static char g_rejected[128];
const char *vk_driver_rejected(void) { return g_rejected; }

void vk_android_load(void) {
    void *lib = NULL;
    const char *drv = getenv("LARECOMP_VK_DRIVER"), *hooks = getenv("LARECOMP_NATIVE_LIB_DIR");
    if (drv && *drv && hooks && *hooks) {
        const char *slash = strrchr(drv, '/');
        char dir[1024];
        if (slash && (size_t)snprintf(dir, sizeof dir, "%.*s/", (int)(slash - drv), drv) < sizeof dir)
            lib = adrenotools_open_libvulkan(RTLD_NOW | RTLD_LOCAL, ADRENOTOOLS_DRIVER_CUSTOM, NULL, hooks, dir, slash + 1,
                                             NULL, NULL);
        LOG("[vk] driver %s: %s\n", drv, lib ? "loaded through adrenotools" : "FAILED, using the system driver");
        PFN_vkGetInstanceProcAddr g = lib ? (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr") : NULL;
        if (lib && (!g || !driver_has_gpu(g))) {
            LOG("[vk] driver %s: no Vulkan 1.1+ GPU it supports here - using the system driver\n", drv);
            lib = NULL;  // (left loaded: a driver doesn't always unload cleanly)
        }
        if (!lib) {  // (the package's name: its folder, files/drivers/<name>/<library>)
            const char *name = drv;
            if (slash) {
                const char *p = slash;
                while (p > drv && p[-1] != '/') p--;
                name = p;
            }
            snprintf(g_rejected, sizeof g_rejected, "%.*s", slash && name < slash ? (int)(slash - name) : (int)strlen(name), name);
        }
    }
    if (!lib) lib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!lib) FATAL("no Vulkan driver: %s\n", dlerror());
    PFN_vkGetInstanceProcAddr gipa = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");
    if (!gipa) FATAL("the Vulkan driver has no vkGetInstanceProcAddr\n");
    volkInitializeCustom(gipa);
}
