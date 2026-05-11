#pragma once

#include <cstdint>

#include "renderer/DrawCall.h"

namespace D {

class IRenderer {
 public:
  virtual ~IRenderer() = default;

  virtual void beginFrame() = 0;
  virtual void submit(const DrawCall&) = 0;
  virtual void endFrame() = 0;
  virtual void resize(uint32_t w, uint32_t h) = 0;
};

}  // namespace D
