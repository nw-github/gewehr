#include <stdafx.h>

#include "utils/math.hpp"

namespace {
    std::pair<float, float> sin_cos(float x) {
        return {sinf(x), cosf(x)};
    }
}

Vector m::angle_vectors(const QAngle& angles) {
    //SinCos(DEG2RAD(angles[1]), &sy, &cy);
    auto [sy, cy] = sin_cos(angles.yaw * 4.0f * atanf(1.0f) / 180.0f);
    //SinCos(DEG2RAD(angles[0]), &sp, &cp);
    auto [sp, cp] = sin_cos(angles.pitch * 4.0f * atanf(1.0f) / 180.0f);
    return Vector{ cp * cy , cp * sy , -sp };
}

float m::get_fov(const QAngle& view_angle, const QAngle& aim_angle) {
    Vector aim = angle_vectors(view_angle);
    Vector ang = angle_vectors(aim_angle);
    return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

QAngle m::calc_angle(const Vector& src, const Vector& dst)
{
    Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
    float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
    QAngle angle(
        atanf(delta.z / hyp) * 57.295779513082f,
        atanf(delta.y / delta.x) * 57.295779513082f,
        0.0f
    );

    if (delta.x >= 0.0)
        angle.yaw += 180.0f;
    return angle;
}

void m::clamp_angles(QAngle &angles)
{
    angles.pitch = std::clamp(angles.pitch, -89.f, 89.f);
    angles.yaw   = std::clamp(angles.yaw, -180.f, 180.f);
    angles.roll  = 0.f;
}
