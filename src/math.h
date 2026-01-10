#pragma once

#include "godot_cpp/core/defs.hpp"
#include "godot_cpp/variant/quaternion.hpp"

using namespace godot;

namespace LNMath {
static _FORCE_INLINE_ Vector3 quat_log(Quaternion q, float eps = 1e-8f) {
	float length = Math::sqrt(q.x * q.x + q.y * q.y + q.z * q.z);

	if (length < eps) {
		return Vector3(q.x, q.y, q.z);
	} else {
		float halfangle = acosf(CLAMP(q.w, -1.0f, 1.0f));
		return halfangle * (Vector3(q.x, q.y, q.z) / length);
	}
}

static _FORCE_INLINE_ Quaternion quat_inv(const Quaternion &p_x) {
	return Quaternion(p_x.x, p_x.y, p_x.z, -p_x.w);
}
static _FORCE_INLINE_ Quaternion quat_abs(const Quaternion &p_x)
{
    return p_x.w < 0.0 ? -p_x : p_x;
}

static _FORCE_INLINE_ Quaternion quat_normalize(Quaternion q, const float eps = 1e-8f) {
	return q / (q.length() + eps);
}

static _FORCE_INLINE_ Quaternion quat_exp(Vector3 v, float eps = 1e-8f) {
	float halfangle = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);

	if (halfangle < eps) {
		return quat_normalize(Quaternion(v.x, v.y, v.z, 1.0f));
	} else {
		float c = cosf(halfangle);
		float s = sinf(halfangle) / halfangle;
		return Quaternion(s * v.x, s * v.y, s * v.z, c);
	}
}

static _FORCE_INLINE_ Quaternion quat_from_scaled_angle_axis(Vector3 v, float eps = 1e-8f) {
	return quat_exp(v / 2.0f, eps);
}

static _FORCE_INLINE_ Vector3 quat_to_scaled_angle_axis(Quaternion q, float eps = 1e-8f) {
	return 2.0f * quat_log(q);
}

static _FORCE_INLINE_ Quaternion quat_mul(Quaternion q, Quaternion p)
{
  return Quaternion(
	  p.w*q.x + p.x*q.w - p.y*q.z + p.z*q.y,
	  p.w*q.y + p.x*q.z + p.y*q.w - p.z*q.x,
	  p.w*q.z - p.x*q.y + p.y*q.x + p.z*q.w,
	  p.w*q.w - p.x*q.x - p.y*q.y - p.z*q.z);
}

static inline void quat_to_angle_axis(Quaternion q, float& angle, Vector3& axis, float eps=1e-8f)
{
    float length = Math::sqrt(q.x*q.x + q.y*q.y + q.z*q.z);

    if (length < eps)
    {
        angle = 0.0f;
        axis = Vector3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        angle = 2.0f * Math::acos(CLAMP(q.w, -1.0f, 1.0f));
        axis = Vector3(q.x, q.y, q.z) / length;
    }
}

static _FORCE_INLINE_ Quaternion quat_from_angle_axis(float angle, Vector3 axis)
{
    float c = cosf(angle / 2.0f);
    float s = sinf(angle / 2.0f);
    return Quaternion(s * axis.x, s * axis.y, s * axis.z, c);
}

static _FORCE_INLINE_ float fast_negexp(float x) {
	return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
}

}; //namespace LNMath