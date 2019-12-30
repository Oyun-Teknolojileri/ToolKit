#pragma once

#include "glm\glm.hpp"
#include "glm\gtc\quaternion.hpp"

/// Assuming transformation applied in this order translate * rotate * scale * vector,
/// Decomposes matrix.
void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation, glm::vec3& scale);
/// Assuming transformation applied in this order translate * rotate * scale * vector,
/// Decomposes matrix.
void DecomposeMatrix(const glm::mat4& transform, glm::vec3& position, glm::quat& rotation);
bool SpherePointIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& vertex);
bool SphereSphereIntersection(const glm::vec3& spherePos, float sphereRadius, const glm::vec3& spherePos2, float sphereRadius2);
glm::vec3 Interpolate(const glm::vec3& vec1, const glm::vec3& vec2, float ratio);
void ToSpherical(glm::vec3 p, float& r, float& zenith, float& azimuth);
glm::vec3 ToCartesian(float r, float zenith, float azimuth);

template <class T>
struct Rect
{
  T x;
  T y;
  T width;
  T height;
};
