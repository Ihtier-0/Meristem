#include "geometry/TurtleBuilder2D.h"

#include <cmath>
#include <stack>

namespace D {

Mesh TurtleBuilder2D::build(const StringStructure& s) {
  Mesh mesh;
  mesh.mode = PrimitiveMode::Lines;
  mesh.positions.reserve(s.derivation.size() * 2);
  mesh.indices.reserve(s.derivation.size() * 2);

  State state{.pos = {0.f, 0.f}, .angle = 90.f};
  std::stack<State> stack;

  auto forward = [&](bool draw) {
    float rad = glm::radians(state.angle);
    glm::vec2 dir = {std::cos(rad), std::sin(rad)};
    glm::vec2 next = state.pos + dir * m_stepLen;

    if (draw) {
      auto base = static_cast<uint32_t>(mesh.positions.size());
      mesh.positions.push_back({state.pos.x, state.pos.y, 0.f});
      mesh.positions.push_back({next.x, next.y, 0.f});
      mesh.indices.push_back(base);
      mesh.indices.push_back(base + 1);
    }

    state.pos = next;
  };

  for (const Symbol& sym : s.derivation) {
    const char c = sym.letter;
    if      (c == m_symbols.forward)       forward(true);
    else if (c == m_symbols.forwardNoDraw) forward(false);
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
