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
  char flower = 'K';   // draws a circle at current position
};

class TurtleBuilder2D : public IGeometryBuilder {
 public:
  explicit TurtleBuilder2D(float angleDeg = 25.f, float stepLen = 1.f)
      : m_angleDeg(angleDeg), m_stepLen(stepLen) {}

  void setAngle(float deg)        { m_angleDeg = deg; }
  void setStep(float len)         { m_stepLen = len; }
  void setSymbols(const TurtleSymbols& s) { m_symbols = s; }
  void setFlowerRadius(float r)   { m_flowerRadius = r; }

  float angle()        const { return m_angleDeg; }
  float step()         const { return m_stepLen; }
  float flowerRadius() const { return m_flowerRadius; }
  const TurtleSymbols& symbols()     const { return m_symbols; }
  const Mesh&          lastFlowerMesh() const { return m_lastFlowers; }

  Mesh build(const Word& word) override;

 private:
  struct State {
    Vec2  pos   = {0.f, 0.f};
    float angle = 90.f;  // degrees, 90 = up
  };

  float m_angleDeg;
  float m_stepLen;
  float m_flowerRadius = 0.3f;
  static constexpr int kFlowerSegments = 16;
  TurtleSymbols m_symbols;
  Mesh m_lastFlowers; // populated by build(), contains flower circles
};

}  // namespace D
