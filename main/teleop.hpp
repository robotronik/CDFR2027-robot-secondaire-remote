#pragma once
#include <cstdint>
#include <cmath>

struct Pose { int16_t x, y, theta; };

inline constexpr int STEP_MM = 100;
inline constexpr int TURN_DEG = 15;

// Wraps an angle in degrees into [-180, 180).
inline int wrap_deg(int a) { return ((a + 180) % 360 + 360) % 360 - 180; }

// Applies one command ('F' 'B' 'L' 'R') to a pose. Unknown commands leave it unchanged.
inline Pose next_target(Pose t, char cmd) {
    constexpr double PI = 3.14159265358979323846;
    double rad = t.theta * PI / 180.0;
    switch (cmd) {
        case 'F':
        case 'B': {
            int s = (cmd == 'F') ? STEP_MM : -STEP_MM;
            t.x = static_cast<int16_t>(t.x + std::lround(s * std::cos(rad)));
            t.y = static_cast<int16_t>(t.y + std::lround(s * std::sin(rad)));
            break;
        }
        case 'L': t.theta = static_cast<int16_t>(wrap_deg(t.theta + TURN_DEG)); break;
        case 'R': t.theta = static_cast<int16_t>(wrap_deg(t.theta - TURN_DEG)); break;
        default: break;
    }
    return t;
}
