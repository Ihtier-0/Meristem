#pragma once

#include <numbers>

namespace D {

// ── Vec2 ──────────────────────────────────────────────────────────────────────

struct Vec2 {
  float x = 0.f, y = 0.f;

  Vec2 operator+(Vec2 v) const noexcept { return {x + v.x, y + v.y}; }
  Vec2 operator-(Vec2 v) const noexcept { return {x - v.x, y - v.y}; }
  Vec2 operator*(float s) const noexcept { return {x * s, y * s}; }
  Vec2& operator+=(Vec2 v) noexcept { x += v.x; y += v.y; return *this; }
};
inline Vec2 operator*(float s, Vec2 v) noexcept { return v * s; }

// ── Vec3 ──────────────────────────────────────────────────────────────────────

struct Vec3 {
  float x = 0.f, y = 0.f, z = 0.f;
};

// ── Vec4 ──────────────────────────────────────────────────────────────────────

struct Vec4 {
  float r = 0.f, g = 0.f, b = 0.f, a = 1.f;
};

// ── Mat4 — column-major (OpenGL / GLM convention) ─────────────────────────────
// data[col * 4 + row], so data[0..3] = column 0, data[4..7] = column 1, …

struct Mat4 {
  float data[16] = {};

  Mat4() noexcept = default;

  // Mat4(1.f) → identity,  Mat4(0.f) → zero matrix
  explicit Mat4(float diag) noexcept {
    data[0] = data[5] = data[10] = data[15] = diag;
  }
};

// ── Quat — stub (reserved for 3D turtle, not yet used) ────────────────────────

struct Quat {
  float x = 0.f, y = 0.f, z = 0.f, w = 1.f;
};

// ── value_ptr — raw float pointer for glUniform* calls ────────────────────────

inline const float* value_ptr(const Mat4& m) noexcept { return m.data; }
inline const float* value_ptr(const Vec4& v) noexcept { return &v.r; }
inline const float* value_ptr(const Vec3& v) noexcept { return &v.x; }
inline const float* value_ptr(const Vec2& v) noexcept { return &v.x; }

// ── Scalar functions ──────────────────────────────────────────────────────────

inline float radians(float deg) noexcept {
  return deg * std::numbers::pi_v<float> / 180.f;
}

// Orthographic projection — column-major, maps [l,r]×[b,t]×[n,f] to [-1,1]³.
// Drop-in replacement for glm::ortho(l, r, b, t, n, f).
inline Mat4 ortho(float l, float r, float b, float t, float n, float f) noexcept {
  Mat4 m;
  m.data[ 0] =  2.f / (r - l);
  m.data[ 5] =  2.f / (t - b);
  m.data[10] = -2.f / (f - n);
  m.data[12] = -(r + l) / (r - l);
  m.data[13] = -(t + b) / (t - b);
  m.data[14] = -(f + n) / (f - n);
  m.data[15] =  1.f;
  return m;
}

}  // namespace D
