// Motion controls: the phone's gyroscope and accelerometer - or a controller's, when it has them - as the Switch
// controller's six-axis sensor (nn::hid::GetSixAxisSensorStates): aiming the bow and the Sheikah Slate's camera, the
// shrines' motion puzzles. Settings > Controls > Motion controls (g_settings.motion) off: the controller lies still.
//
// Frames: SDL gives a sensor as a device held in front of you - x right, y up, z toward you (a phone in its natural
// portrait: turned to the screen here, the game being in landscape). The Switch's six-axis frame is x right, y
// forward, z up (yuzu's mapping of SDL controllers): angular velocity (x, -z, y) in turns per second, acceleration
// -(x, -z, y) in G (at rest it points down). The direction matrix follows the angular velocity, kept level by the
// acceleration (a Mahony filter): its world is z up, with the yaw the sensor started at.
#include <SDL3/SDL.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "../hle.h"
#include "../settings.h"

typedef struct {
    int64_t delta_time;   // ns since the sample before
    int64_t sampling_number;
    float accel[3];       // G
    float gyro[3];        // turns per second
    float angle[3];       // turns: the angular velocity summed
    float dir[3][3];      // the sensor's x, y and z axes in the world (lying still, the right way up: identity)
    uint32_t attributes;  // 1: connected
    uint32_t reserved;
} SixAxisState;
_Static_assert(sizeof(SixAxisState) == 0x60, "nn::hid::SixAxisSensorState");

#define RING 32
#define KP 0.5f       // the acceleration's pull on the attitude (rad/s per unit of error)
#define TURN 6.2831853f
static pthread_mutex_t m_lock = PTHREAD_MUTEX_INITIALIZER;
static SixAxisState ring[RING];
static int ring_n, ring_head;            // samples kept, the newest's index
static int64_t m_sampling;
static float q[4] = {1, 0, 0, 0};        // w x y z: the sensor's attitude (sensor -> world)
static bool q_set;                       // (levelled by the first acceleration)
static float m_accel[3] = {0, 0, -1}, m_angle[3];
static uint64_t m_gyro_ts, m_live_ns, m_rest_ns;  // the last angular velocity's own time; host times
static uint64_t pad_until_ns;            // a controller's sensors are in use until then: the phone's wait

// q from the way down a (in G): no yaw - the rotation taking a to the world's (0, 0, -1)
static void level_from(const float a[3]) {
    float n = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (n < 0.5f) return;
    float ux = a[0] / n, uy = a[1] / n, uz = a[2] / n;
    float w = 1 - uz, x = -uy, y = ux;  // (1 + u.v, u x v) with v = (0, 0, -1)
    float m = sqrtf(w * w + x * x + y * y);
    if (m < 1e-4f) q[0] = 0, q[1] = 1, q[2] = 0, q[3] = 0;  // (upside down: half a turn about x)
    else q[0] = w / m, q[1] = x / m, q[2] = y / m, q[3] = 0;
    q_set = true;
}

// One step of the attitude: w (rad/s, the sensor's axes), pulled toward the measured down a
static void fuse(float w[3], const float a[3], float dt) {
    float qw = q[0], x = q[1], y = q[2], z = q[3];
    float n = sqrtf(a[0] * a[0] + a[1] * a[1] + a[2] * a[2]);
    if (n > 0.75f && n < 1.25f) {  // (not while shaken)
        // the world's down seen from the sensor: -(R's third row)
        float vx = -2 * (x * z - qw * y), vy = -2 * (y * z + qw * x), vz = -(1 - 2 * (x * x + y * y));
        float ax = a[0] / n, ay = a[1] / n, az = a[2] / n;
        w[0] += KP * (ay * vz - az * vy);
        w[1] += KP * (az * vx - ax * vz);
        w[2] += KP * (ax * vy - ay * vx);
    }
    q[0] += 0.5f * dt * (-x * w[0] - y * w[1] - z * w[2]);
    q[1] += 0.5f * dt * (qw * w[0] + y * w[2] - z * w[1]);
    q[2] += 0.5f * dt * (qw * w[1] - x * w[2] + z * w[0]);
    q[3] += 0.5f * dt * (qw * w[2] + x * w[1] - y * w[0]);
    float m = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    for (int i = 0; i < 4; i++) q[i] /= m;
}

// A sample: angular velocity w (rad/s, the Switch's frame) over dt seconds (m_lock held)
static void ring_push(const float w[3], float dt);
static void add_sample(const float w[3], float dt) {
    if (!q_set) level_from(m_accel);
    float wc[3] = {w[0], w[1], w[2]};
    fuse(wc, m_accel, dt);
    ring_push(w, dt);
}
// (S23) The sample of the attitude q as it is now and the angular velocity w (rad/s) into the ring (m_lock held)
static void ring_push(const float w[3], float dt) {
    ring_head = (ring_head + 1) % RING;
    if (ring_n < RING) ring_n++;
    SixAxisState *s = &ring[ring_head];
    memset(s, 0, sizeof *s);
    s->delta_time = (int64_t)(dt * 1e9f);
    s->sampling_number = ++m_sampling;
    for (int i = 0; i < 3; i++) {
        s->accel[i] = m_accel[i];
        s->gyro[i] = w[i] / TURN;
        m_angle[i] += s->gyro[i] * dt;
        s->angle[i] = m_angle[i];
    }
    float qw = q[0], x = q[1], y = q[2], z = q[3];
    float r[3][3] = {{1 - 2 * (y * y + z * z), 2 * (x * y - qw * z), 2 * (x * z + qw * y)},
                     {2 * (x * y + qw * z), 1 - 2 * (x * x + z * z), 2 * (y * z - qw * x)},
                     {2 * (x * z - qw * y), 2 * (y * z + qw * x), 1 - 2 * (x * x + y * y)}};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) s->dir[i][j] = r[j][i];  // (row i: the sensor's axis i, a column of R)
    s->attributes = 1;
}

// SDL's frame (rad/s or m/s^2) in: angular velocity -> a sample, acceleration -> the way down
extern volatile bool g_stick_motion;
static void feed(SDL_SensorType type, const float d[3], uint64_t ts) {
    pthread_mutex_lock(&m_lock);
    if (g_stick_motion) { pthread_mutex_unlock(&m_lock); return; }  // (S23: the sticks make the motion now)
    if (type == SDL_SENSOR_ACCEL) {
        m_accel[0] = -d[0] / 9.80665f, m_accel[1] = d[2] / 9.80665f, m_accel[2] = -d[1] / 9.80665f;
        if (!q_set) level_from(m_accel);
    } else if (type == SDL_SENSOR_GYRO) {
        float dt = m_gyro_ts && ts > m_gyro_ts ? (float)((ts - m_gyro_ts) / 1e9) : 0.005f;
        if (dt > 0.1f) dt = 0.005f;  // (after a pause)
        m_gyro_ts = ts;
        // The gyroscope's bias at rest, learned and taken off: a phone's reads a little turn while it lies still, and the
        // aim drifted (SavageWindRecomp / MacNativeGames: learn it while still). Still: every axis within ~3.4 deg/s of
        // the bias for 1.5 s; then the bias follows slowly. LARECOMP_NO_GYRO_BIAS=1: off.
        static int bias_on = -1;
        static float bias[3];
        static uint64_t still_since;
        if (bias_on < 0) bias_on = getenv("LARECOMP_NO_GYRO_BIAS") == NULL;
        float g[3] = {d[0], d[1], d[2]};
        if (bias_on) {
            float dev = 0;
            for (int i = 0; i < 3; i++) dev = fmaxf(dev, fabsf(d[i] - bias[i]));
            if (dev < 0.06f) {
                if (!still_since) still_since = ts;
                else if (ts - still_since > 1500000000ull) {
                    for (int i = 0; i < 3; i++) bias[i] += (d[i] - bias[i]) * 0.02f;
                    static bool told;
                    if (!told && ts - still_since > 4000000000ull) {
                        told = true;
                        LOG("[motion] gyroscope at rest: bias %.4f %.4f %.4f rad/s taken off\n", bias[0], bias[1], bias[2]);
                    }
                }
            } else {
                still_since = 0;
            }
            for (int i = 0; i < 3; i++) g[i] -= bias[i];
        }
        const float w[3] = {g[0], -g[2], g[1]};
        add_sample(w, dt);
        m_live_ns = SDL_GetTicksNS();
    }
    pthread_mutex_unlock(&m_lock);
}

// The phone's sensors (device axes, portrait) on the screen as it's turned now
static void to_screen(const float d[3], float s[3]) {
    switch (SDL_GetCurrentDisplayOrientation(SDL_GetPrimaryDisplay())) {
    case SDL_ORIENTATION_LANDSCAPE: s[0] = -d[1], s[1] = d[0]; break;          // (its right side up)
    case SDL_ORIENTATION_LANDSCAPE_FLIPPED: s[0] = d[1], s[1] = -d[0]; break;  // (its left side up)
    case SDL_ORIENTATION_PORTRAIT_FLIPPED: s[0] = -d[0], s[1] = -d[1]; break;
    default: s[0] = d[0], s[1] = d[1]; break;
    }
    s[2] = d[2];
}

// (the event thread: runtime/hle/input.c) The phone's gyroscope and accelerometer, once the setting is on
void motion_open(void) {
    static bool opened;
    if (opened || !g_settings.motion) return;
    opened = true;
    if (!SDL_InitSubSystem(SDL_INIT_SENSOR)) { LOG("[motion] sensors: %s\n", SDL_GetError()); return; }
    int n = 0;
    SDL_SensorID *ids = SDL_GetSensors(&n);
    bool gyro = false, accel = false;
    for (int i = 0; i < n; i++) {
        SDL_SensorType t = SDL_GetSensorTypeForID(ids[i]);
        if ((t == SDL_SENSOR_GYRO && !gyro) || (t == SDL_SENSOR_ACCEL && !accel)) {
            if (!SDL_OpenSensor(ids[i])) continue;
            if (t == SDL_SENSOR_GYRO) gyro = true;
            else accel = true;
            LOG("[motion] %s: %s\n", t == SDL_SENSOR_GYRO ? "gyroscope" : "accelerometer", SDL_GetSensorNameForID(ids[i]));
        }
    }
    SDL_free(ids);
    if (!gyro) LOG("[motion] no gyroscope among %d sensors: motion controls only from a controller that has one\n", n);
}

// A controller that has a gyroscope: its own sensors, in place of the phone's while it sends them
void motion_pad_opened(SDL_Gamepad *g) {
    if (!SDL_GamepadHasSensor(g, SDL_SENSOR_GYRO)) return;
    SDL_SetGamepadSensorEnabled(g, SDL_SENSOR_GYRO, true);
    if (SDL_GamepadHasSensor(g, SDL_SENSOR_ACCEL)) SDL_SetGamepadSensorEnabled(g, SDL_SENSOR_ACCEL, true);
    LOG("[motion] %s: its gyroscope\n", SDL_GetGamepadName(g));
}

void motion_event(const SDL_Event *e) {
    if (!g_settings.motion || ENV("LARECOMP_MOTION_TEST")) return;
    if (e->type == SDL_EVENT_SENSOR_UPDATE) {
        if (SDL_GetTicksNS() < pad_until_ns) return;
        float s[3];
        to_screen(e->sensor.data, s);
        feed(SDL_GetSensorTypeForID(e->sensor.which), s, e->sensor.sensor_timestamp ? e->sensor.sensor_timestamp : SDL_GetTicksNS());
    } else if (e->type == SDL_EVENT_GAMEPAD_SENSOR_UPDATE) {
        // A controller's sensors replace the phone's only once its gyroscope has moved: some report one that never
        // does (a USB-C controller the phone sits in, GameSir X5 Lite: issue #78), and its zeros held the phone's off
        static bool pad_live;
        if (!pad_live && (SDL_SensorType)e->gsensor.sensor == SDL_SENSOR_GYRO) {
            const float *g = e->gsensor.data;
            if (fabsf(g[0]) + fabsf(g[1]) + fabsf(g[2]) > 0.02f) {
                pad_live = true;
                LOG("[motion] the controller's gyroscope moves: its sensors in place of the phone's\n");
            }
        }
        if (!pad_live) return;
        pad_until_ns = SDL_GetTicksNS() + 500000000;
        feed((SDL_SensorType)e->gsensor.sensor, e->gsensor.data,
             e->gsensor.sensor_timestamp ? e->gsensor.sensor_timestamp : SDL_GetTicksNS());
    }
}

// LARECOMP_MOTION_TEST=gx,gy,gz[:from:frames]: that angular velocity (turns per second, the Switch's frame) in place
// of the sensors', from that presented frame on for that many (all along without them)
static bool test_motion(float w[3]) {
    const char *e = ENV("LARECOMP_MOTION_TEST");
    if (!e) return false;
    float g[3] = {0};
    unsigned long from = 0, len = ~0ul;
    if (sscanf(e, "%f,%f,%f:%lu:%lu", &g[0], &g[1], &g[2], &from, &len) < 3) return false;
    extern uint64_t renderer_present_count(void);
    uint64_t f = renderer_present_count();
    for (int i = 0; i < 3; i++) w[i] = f >= from && f - from < len ? g[i] * TURN : 0;
    return true;
}

// A read with nothing new from the sensors (none, the setting off, or a test): one sample of its own
static void own_sample(uint64_t now) {
    float dt = m_rest_ns && now > m_rest_ns ? (float)((now - m_rest_ns) / 1e9) : 1 / 30.0f;
    if (dt > 0.1f) dt = 1 / 30.0f;
    m_rest_ns = now;
    float w[3] = {0, 0, 0};
    if (!test_motion(w) && !g_settings.motion) {  // (off: lying still, the right way up)
        q[0] = 1, q[1] = q[2] = q[3] = 0, q_set = true;
        m_accel[0] = m_accel[1] = 0, m_accel[2] = -1;
    }
    add_sample(w, dt);
}

// ---- (S23) Motion with the sticks (the menu's System tab > Controls; not saved: off at every start of the app). For the
// shrines' motion puzzles, where tilting the phone just right was hard work: the left stick tilts the controller, both ways
// at once, up to 60 degrees, and back to flat when let go - as tilting a Switch controller in the hands; the right stick
// turns it further and leaves it there, half a turn a second at full tilt - to turn a puzzle upside down. The two add up:
// the left stick's tilt from where the right one left it. Meanwhile the phone's and the controller's own sensors are left
// out (feed), and the game gets the sticks centred (hle/input.c): they move neither the player nor the camera.
// LARECOMP_STICK_MOTION_SIGN=-1: both sticks the other way round.
volatile bool g_stick_motion;
static _Atomic int32_t sm_lx, sm_ly, sm_rx, sm_ry;
void motion_sticks(int32_t lx, int32_t ly, int32_t rx, int32_t ry) { sm_lx = lx, sm_ly = ly, sm_rx = rx, sm_ry = ry; }
static float sm_base[4] = {1, 0, 0, 0};  // the right stick's attitude (w x y z)
static float sm_tilt[2];                 // the left stick's tilt, radians about x and y (followed smoothly)
static bool sm_was;
static void qmul(const float a[4], const float b[4], float o[4]) {
    float r[4] = {a[0] * b[0] - a[1] * b[1] - a[2] * b[2] - a[3] * b[3], a[0] * b[1] + a[1] * b[0] + a[2] * b[3] - a[3] * b[2],
                  a[0] * b[2] - a[1] * b[3] + a[2] * b[0] + a[3] * b[1], a[0] * b[3] + a[1] * b[2] - a[2] * b[1] + a[3] * b[0]};
    memcpy(o, r, sizeof r);
}
static void qaxis(float ax, float ay, float az, float ang, float o[4]) {  // ang radians about the unit axis (ax, ay, az)
    float s = sinf(ang * 0.5f);
    o[0] = cosf(ang * 0.5f), o[1] = ax * s, o[2] = ay * s, o[3] = az * s;
}
// -1..1 past a 10% dead zone, on a curve: gentle through most of the travel (half way: a quarter of the motion), the
// whole of it only at the stick's edge - fine moves for the puzzles, the big ones still there when pushed all the way
static float stick_unit(int32_t v) {
    float f = v / 32767.0f, a = fabsf(f);
    if (a < 0.1f) return 0;
    f = (a - 0.1f) / 0.9f * (f < 0 ? -1.0f : 1.0f);
    f = f > 1 ? 1 : f < -1 ? -1 : f;
    return 0.3f * f + 0.7f * f * f * f;
}
// The Switch's frame: x right, y forward, z up; the sticks' y is up / forward. Left stick forward: the far edge down
// (about x, negative); right: the right edge down (about y, positive). The right stick turns about the same two axes.
static void stick_sample(uint64_t now) {  // (m_lock held)
    float dt = m_rest_ns && now > m_rest_ns ? (float)((now - m_rest_ns) / 1e9) : 1 / 30.0f;
    if (dt > 0.1f) dt = 1 / 30.0f;
    m_rest_ns = now;
    static int sign;
    if (!sign) sign = getenv("LARECOMP_STICK_MOTION_SIGN") && atoi(getenv("LARECOMP_STICK_MOTION_SIGN")) < 0 ? -1 : 1;
    float lx = sign * stick_unit(sm_lx), ly = sign * stick_unit(sm_ly), rx = sign * stick_unit(sm_rx), ry = sign * stick_unit(sm_ry);
    if (rx || ry) {
        const float rate = 3.14159265f;  // (rad/s at full tilt: half a turn a second)
        float wx = -ry * rate, wy = rx * rate, m = sqrtf(wx * wx + wy * wy), d[4];
        qaxis(wx / m, wy / m, 0, m * dt, d);
        qmul(sm_base, d, sm_base);
        float n = sqrtf(sm_base[0] * sm_base[0] + sm_base[1] * sm_base[1] + sm_base[2] * sm_base[2] + sm_base[3] * sm_base[3]);
        for (int i = 0; i < 4; i++) sm_base[i] /= n;
    }
    const float maxt = 1.0471976f;  // (60 degrees)
    float k = fminf(1.0f, dt * 10);  // (the tilt follows the stick in about a tenth of a second: no jumps)
    sm_tilt[0] += (-ly * maxt - sm_tilt[0]) * k;
    sm_tilt[1] += (lx * maxt - sm_tilt[1]) * k;
    float tx[4], ty[4], t[4], nq[4];
    qaxis(1, 0, 0, sm_tilt[0], tx);
    qaxis(0, 1, 0, sm_tilt[1], ty);
    qmul(tx, ty, t);
    qmul(sm_base, t, nq);
    // the angular velocity from the last attitude to this one, in the sensor's own axes
    float inv[4] = {q[0], -q[1], -q[2], -q[3]}, dq[4];
    qmul(inv, nq, dq);
    if (dq[0] < 0) for (int i = 0; i < 4; i++) dq[i] = -dq[i];
    float w[3] = {2 * dq[1] / dt, 2 * dq[2] / dt, 2 * dq[3] / dt};
    memcpy(q, nq, sizeof nq);
    q_set = true;
    // the way down seen from the sensor, as an accelerometer at rest reads it (fuse's -(R's third row))
    m_accel[0] = -2 * (q[1] * q[3] - q[0] * q[2]);
    m_accel[1] = -2 * (q[2] * q[3] + q[0] * q[1]);
    m_accel[2] = -(1 - 2 * (q[1] * q[1] + q[2] * q[2]));
    ring_push(w, dt);
}
static void stick_mode_check(void) {  // (m_lock held) turned on: flat, as a controller lying still; off: levelled again
    if (g_stick_motion == sm_was) return;
    sm_was = g_stick_motion;
    sm_base[0] = 1, sm_base[1] = sm_base[2] = sm_base[3] = 0, sm_tilt[0] = sm_tilt[1] = 0;
    if (sm_was) q[0] = 1, q[1] = q[2] = q[3] = 0, q_set = true, m_accel[0] = m_accel[1] = 0, m_accel[2] = -1;
    else q_set = false;
    LOG("[motion] motion with the sticks %s\n", sm_was ? "on: the phone's and the controller's sensors left out" : "off");
}

// int GetSixAxisSensorHandles(SixAxisSensorHandle *out, int count, const NpadIdType &, NpadStyleSet): one sensor, two
// for a pair of Joy-Con (both read the same here)
HLE_FUNC("_ZN2nn3hid23GetSixAxisSensorHandlesEPNS0_19SixAxisSensorHandleEiRKjNS_4util10BitFlagSetILi32ENS0_12NpadStyleTagEEE",
         hid_sixaxis_handles) {
    uint32_t *out = (uint32_t *)(uintptr_t)c->x[0];
    int n = (int)(int32_t)c->x[1];
    uint32_t id = *(uint32_t *)(uintptr_t)c->x[2], style = (uint32_t)c->x[3];
    int k = style & 4 ? 2 : 1;  // (JoyDual: left and right)
    if (n > k) n = k;
    if (n < 0) n = 0;
    for (int i = 0; i < n; i++) out[i] = (uint32_t)i << 16 | (id & 0xFF) << 8 | 3;  // device, player, FullKey
    static int nlog;
    if (nlog++ < 4) LOG("[motion] sensor handles: player %#x style %#x -> %d\n", id, style, n);
    c->x[0] = (uint64_t)n;
}

HLE_FUNC("_ZN2nn3hid18StartSixAxisSensorERKNS0_19SixAxisSensorHandleE", hid_sixaxis_start) {
    static int nlog;
    if (nlog++ < 4) LOG("[motion] sensor %#x started\n", *(uint32_t *)(uintptr_t)c->x[0]);
}

HLE_FUNC("_ZN2nn3hid25SetGyroscopeZeroDriftModeERKNS0_19SixAxisSensorHandleERKNS0_22GyroscopeZeroDriftModeE",
         hid_sixaxis_drift) { (void)c; }

// int GetSixAxisSensorStates(SixAxisSensorState *out, int count, const SixAxisSensorHandle &): the newest first
HLE_FUNC("_ZN2nn3hid22GetSixAxisSensorStatesEPNS0_18SixAxisSensorStateEiRKNS0_19SixAxisSensorHandleE", hid_sixaxis_states) {
    SixAxisState *out = (SixAxisState *)(uintptr_t)c->x[0];
    int n = (int)(int32_t)c->x[1];
    uint64_t now = SDL_GetTicksNS();
    pthread_mutex_lock(&m_lock);
    stick_mode_check();
    if (g_stick_motion) stick_sample(now);
    else if (!g_settings.motion || ENV("LARECOMP_MOTION_TEST") || now - m_live_ns > 250000000) own_sample(now);
    if (n > ring_n) n = ring_n;
    if (n < 0) n = 0;
    for (int i = 0; i < n; i++) out[i] = ring[(ring_head - i + RING) % RING];
    static uint64_t logged;
    if (ENV("LARECOMP_LOG_MOTION") && now - logged > 1000000000) {  // (once a second: the newest sample)
        logged = now;
        const SixAxisState *s = &ring[ring_head];
        LOG("[motion] #%lld dt %.1f ms accel %.2f %.2f %.2f gyro %.3f %.3f %.3f angle %.3f %.3f %.3f y-axis %.2f %.2f %.2f "
            "(%d of %d)\n", (long long)s->sampling_number, s->delta_time / 1e6, s->accel[0], s->accel[1], s->accel[2],
            s->gyro[0], s->gyro[1], s->gyro[2], s->angle[0], s->angle[1], s->angle[2], s->dir[1][0], s->dir[1][1],
            s->dir[1][2], n, (int)(int32_t)c->x[1]);
    }
    pthread_mutex_unlock(&m_lock);
    c->x[0] = (uint64_t)n;
}

RT_ON_RESET(motion_reset) {
    pthread_mutex_lock(&m_lock);
    ring_n = 0, q_set = false, m_rest_ns = 0;
    memset(m_angle, 0, sizeof m_angle);
    pthread_mutex_unlock(&m_lock);
}
