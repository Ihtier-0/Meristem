#pragma once

#include "structure/StringStructure.h"

namespace D {

class IPlantAlgorithm {
 public:
  virtual ~IPlantAlgorithm() = default;

  virtual void step() = 0;
  virtual void reset() = 0;
  virtual int generation() const = 0;
  virtual const StringStructure& getStructure() const = 0;
};

}  // namespace D
