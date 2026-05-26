#pragma once

#include "core/types.h"

namespace D {

class IPlantAlgorithm {
 public:
  virtual ~IPlantAlgorithm() = default;

  virtual void step() = 0;
  virtual void reset() = 0;
  virtual int generation() const = 0;
  virtual const Word& current() const = 0;
};

}  // namespace D
