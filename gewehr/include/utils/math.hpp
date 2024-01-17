#pragma once

class QAngle;
class Vector;
struct matrix4x4_t;

#define M_PI       3.14159265358979323846f
#define RAD2DEG(x) ((float)(x) * (180.f / M_PI))
#define DEG2RAD(x) ((float)(x) * (M_PI / 180.f))

namespace m
{
    template<typename T>
    inline void normalize3(T &vec)
    {
        for (auto i = 0; i < 2; i++)
        {
            while (vec[i] < -180.0f)
                vec[i] += 360.0f;
            while (vec[i] > 180.0f)
                vec[i] -= 360.0f;
        }

        vec[2] = 0.f;
    }

    Vector angle_vectors(const QAngle& angles);
    float get_fov(const QAngle& view_angle, const QAngle& aim_angle);
    QAngle calc_angle(const Vector& src, const Vector& dst);
    void clamp_angles(QAngle& angles);
    // std::optional<Vector> world_to_screen(const Vector& origin, const matrix4x4_t& wts_matrix);
}