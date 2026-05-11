#pragma once

#include "structure/PlantStructure.h"

namespace D {

class IPlantAlgorithm {
 public:
  virtual ~IPlantAlgorithm() = default;

  virtual void step() = 0;
  virtual void reset() = 0;
  virtual int generation() const = 0;
  virtual const PlantStructure& getStructure() const = 0;
};

}  // namespace D
