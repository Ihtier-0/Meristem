#pragma once

#include "geometry/IGeometryBuilder.h"

namespace D {

struct TurtleSymbols {
  char forward = 'F';
  char forwardNoDraw = 'f';
  char turnLeft = '+';
  char turnRight = '-';
  char turnAround = '|';
  char push = '[';
  char pop = ']';
};

class TurtleBuilder2D : public IGeometryBuilder {
 public:
  explicit TurtleBuilder2D(float angleDeg = 25.f, float stepLen = 1.f)
      : m_angleDeg(angleDeg), m_stepLen(stepLen) {}

  void setAngle(float deg) { m_angleDeg = deg; }
  void setStep(float len) { m_stepLen = len; }
  void setSymbols(const TurtleSymbols& s) { m_symbols = s; }

  float angle() const { return m_angleDeg; }
  float step() const { return m_stepLen; }
  const TurtleSymbols& symbols() const { return m_symbols; }

  Mesh build(const StringStructure& s) override;
  Mesh build(const TreeGraph&) override { return {}; }

 private:
  struct State {
    glm::vec2 pos = {0.f, 0.f};
    float angle = 90.f;  // degrees, 90 = up
  };

  float m_angleDeg;
  float m_stepLen;
  TurtleSymbols m_symbols;
};

}  // namespace D
