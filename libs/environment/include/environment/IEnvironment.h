#pragma once

#include "core/types.h"

namespace D {

struct EnvironmentSample {
  Vec3 lightDir = {0.f, 1.f, 0.f};
  float lightIntensity = 1.f;
  float humidity = 0.5f;
};

class IEnvironment {
 public:
  virtual ~IEnvironment() = default;

  virtual Vec3 lightAt(Vec3 pos) const = 0;
  virtual Vec3 gravityDir() const = 0;
  virtual EnvironmentSample query(Vec3 pos, float radius) const = 0;
};

}  // namespace D
