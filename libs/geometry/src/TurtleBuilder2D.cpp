#include "geometry/TurtleBuilder2D.h"

#include <cmath>
#include <numbers>
#include <stack>

namespace D {

Mesh TurtleBuilder2D::build(const Word& word) {
  Mesh mesh;
  mesh.mode = PrimitiveMode::Lines;

  m_lastFlowers = Mesh{};
  m_lastFlowers.mode = PrimitiveMode::Lines;
  mesh.positions.reserve(word.size() * 2);
  mesh.indices.reserve(word.size() * 2);

  State state{.pos = {0.f, 0.f}, .angle = 90.f};
  std::stack<State> stack;

  auto forward = [&](bool draw, float lenScale = 1.f) {
    float rad = radians(state.angle);
    Vec2  dir  = {std::cos(rad), std::sin(rad)};
    Vec2  next = state.pos + dir * m_stepLen * lenScale;

    if (draw) {
      auto base = static_cast<uint32_t>(mesh.positions.size());
      mesh.positions.push_back({state.pos.x, state.pos.y, 0.f});
      mesh.positions.push_back({next.x, next.y, 0.f});
      mesh.indices.push_back(base);
      mesh.indices.push_back(base + 1);
    }

    state.pos = next;
  };

  for (const Symbol& sym : word) {
    const char c = sym.letter;
    if (c == m_symbols.forward) {
      float scale = sym.params.empty() ? 1.f
                  : std::visit([](auto v){ return static_cast<float>(v); }, sym.params[0]);
      forward(true, scale);
    } else if (c == m_symbols.forwardNoDraw) {
      float scale = sym.params.empty() ? 1.f
                  : std::visit([](auto v){ return static_cast<float>(v); }, sym.params[0]);
      forward(false, scale);
    } else if (c == m_symbols.flower) {
      // 1. draw a short stem in the current heading (into main mesh)
      float stemLen = m_stepLen * 0.6f;
      float rad0       = radians(state.angle);
      Vec2  flowerPos  = state.pos + Vec2{std::cos(rad0), std::sin(rad0)} * stemLen;
      {
        auto base = static_cast<uint32_t>(mesh.positions.size());
        mesh.positions.push_back({state.pos.x,  state.pos.y,  0.f});
        mesh.positions.push_back({flowerPos.x,  flowerPos.y,  0.f});
        mesh.indices.push_back(base);
        mesh.indices.push_back(base + 1);
      }
      // 2. draw circle approximation into flower mesh
      const float r = m_flowerRadius;
      const float step = 2.f * std::numbers::pi_v<float> / kFlowerSegments;
      for (int k = 0; k < kFlowerSegments; ++k) {
        float a0 = step * k;
        float a1 = step * (k + 1);
        Vec2 p0 = flowerPos + Vec2{std::cos(a0), std::sin(a0)} * r;
        Vec2 p1 = flowerPos + Vec2{std::cos(a1), std::sin(a1)} * r;
        auto base = static_cast<uint32_t>(m_lastFlowers.positions.size());
        m_lastFlowers.positions.push_back({p0.x, p0.y, 0.f});
        m_lastFlowers.positions.push_back({p1.x, p1.y, 0.f});
        m_lastFlowers.indices.push_back(base);
        m_lastFlowers.indices.push_back(base + 1);
      }
      state.pos = flowerPos;
    }
    else if (c == m_symbols.turnLeft)      state.angle += m_angleDeg;
    else if (c == m_symbols.turnRight)     state.angle -= m_angleDeg;
    else if (c == m_symbols.turnAround)    state.angle += 180.f;
    else if (c == m_symbols.push)          stack.push(state);
    else if (c == m_symbols.pop && !stack.empty()) {
      state = stack.top();
      stack.pop();
    }
  }

  return mesh;
}

}  // namespace D
