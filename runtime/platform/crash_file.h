// (S23) A crash's report in a file of its own, always on: the log only reaches a file with the setting on, stderr goes
// through a pipe the dying process may never drain, and Android's logcat had already dropped a SIGSEGV of 2026-10-29 by
// the morning after. Appended, so a later run doesn't replace it (at most CRASH_FILE_MAX bytes, then the oldest stay).
// write() straight to the file; memory read through process_vm_readv, so a bad frame pointer ends the walk instead of
// faulting inside the handler.
#pragma once
#include <dlfcn.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define CRASH_FILE_MAX (1 << 20)
#define CRASH_FRAMES 32

static bool crash_read(uint64_t addr, void *out, size_t n) {
    struct iovec local = {out, n}, remote = {(void *)(uintptr_t)addr, n};
    return process_vm_readv(getpid(), &local, 1, &remote, 1, 0) == (ssize_t)n;
}

static void crash_line(int fd, const char *fmt, unsigned long long a, unsigned long long b) {
    char line[256];
    int n = snprintf(line, sizeof line, fmt, a, b);
    if (n > 0 && write(fd, line, (size_t)n) < 0) return;
}

// One address with the module it's in ("?": no module - the game's own code, or nothing mapped)
static void crash_where(int fd, const char *tag, uint64_t a) {
    Dl_info di = {0};
    char line[512];
    const char *mod = "?";
    uint64_t off = 0;
    if (dladdr((void *)(uintptr_t)a, &di) && di.dli_fname) {
        const char *s = strrchr(di.dli_fname, '/');
        mod = s ? s + 1 : di.dli_fname;
        off = a - (uint64_t)(uintptr_t)di.dli_fbase;
    }
    int n = snprintf(line, sizeof line, "%s %016llx %s+%llx\n", tag, (unsigned long long)a, mod, (unsigned long long)off);
    if (n > 0 && write(fd, line, (size_t)n) < 0) return;
}

// regs: nregs general registers, printed as they are. fp: where the frame chain starts ({caller fp, return address}).
static void crash_file_write(const char *path, int sig, int code, uint64_t fault, uint64_t pc, uint64_t lr, uint64_t sp,
                             uint64_t fp, const uint64_t *regs, int nregs) {
    struct stat st;
    if (stat(path, &st) == 0 && st.st_size > CRASH_FILE_MAX) return;
    int fd = open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0660);
    if (fd < 0) return;
    char head[256], thread[17] = {0};
    prctl(PR_GET_NAME, thread);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm;
    localtime_r(&ts.tv_sec, &tm);
    int n = snprintf(head, sizeof head, "\n=== %04d-%02d-%02d %02d:%02d:%02d signal %d code %d, thread \"%s\" (%d)\n",
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, sig, code, thread,
                     gettid());
    if (n > 0 && write(fd, head, (size_t)n) < 0) { close(fd); return; }
    crash_line(fd, "fault address %016llx, sp %016llx\n", fault, sp);
    crash_where(fd, "pc", pc);
    crash_where(fd, "lr", lr);
    for (int i = 0; i + 1 < nregs; i += 2) {
        char tag[32];
        snprintf(tag, sizeof tag, "r%-2d %%016llx  r%-2d %%016llx\n", i, i + 1);
        crash_line(fd, tag, regs[i], regs[i + 1]);
    }
    for (int i = 0; i < CRASH_FRAMES && fp; i++) {
        uint64_t f[2];  // {caller's frame pointer, return address}
        if (fp & 7 || !crash_read(fp, f, sizeof f) || !f[1]) break;
        char tag[16];
        snprintf(tag, sizeof tag, "#%-2d", i);
        crash_where(fd, tag, f[1] & 0x0000ffffffffffffull);  // (without a pointer-authentication signature)
        if (f[0] <= fp) break;  // (frames go up the stack)
        fp = f[0];
    }
    close(fd);
}
