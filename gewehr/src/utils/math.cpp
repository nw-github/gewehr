#include <stdafx.h>

#include "utils/math.hpp"

#include "utils/render.hpp"

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

float m::get_fov(const QAngle& view_angle, const QAngle& aim_angle)
{
    Vector aim = angle_vectors(view_angle);
    Vector ang = angle_vectors(aim_angle);
    return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}

QAngle m::calc_angle(const Vector& src, const Vector& dst)
{
    Vector delta((src.x - dst.x), (src.y - dst.y), (src.z - dst.z));
    float hyp = sqrt(delta.x * delta.x + delta.y * delta.y);
    QAngle vAngle(
        atanf(delta.z / hyp) * 57.295779513082f,
        atanf(delta.y / delta.x) * 57.295779513082f,
        0.0f
    );

    if (delta.x >= 0.0)
        vAngle.yaw += 180.0f;

    return vAngle;
}

void m::clamp_angles(QAngle &angles)
{
    angles.pitch = std::clamp(angles.pitch, -89.f, 89.f);
    angles.yaw   = std::clamp(angles.yaw, -180.f, 180.f);
    angles.roll  = 0.f;
}

bool m::world_to_screen(const Vector& origin, Vector& screen, const matrix4x4_t& wts_matrix)
{
    const float width = wts_matrix[3][0] * origin.x + wts_matrix[3][1] * origin.y + wts_matrix[3][2] * origin.z + wts_matrix[3][3];
    if (width < 0.001f)
        return false;

    // compute the scene coordinates of a point in 3d
    const float inverse = 1.0f / width;
    screen.x = (wts_matrix[0][0] * origin.x + wts_matrix[0][1] * origin.y + wts_matrix[0][2] * origin.z + wts_matrix[0][3]) * inverse;
    screen.y = (wts_matrix[1][0] * origin.x + wts_matrix[1][1] * origin.y + wts_matrix[1][2] * origin.z + wts_matrix[1][3]) * inverse;

    // screen transform
    // get the screen position in pixels of given point
    screen.x = (g::render->get_resolution().x * 0.5f) + (screen.x * g::render->get_resolution().x) * 0.5f;
    screen.y = (g::render->get_resolution().y * 0.5f) - (screen.y * g::render->get_resolution().y) * 0.5f;
    return true;
}