#pragma once

#include <string>
#include <vector>

#include <glm/vec4.hpp>

#include "algorithm/D0LSystemAlgorithm.h"
#include "geometry/TurtleBuilder2D.h"
#include "geometry/Mesh.h"
#include "renderer/OpenGLRenderer.h"

namespace D {

class ViewerUI {
 public:
  ViewerUI(LSystemGrammar& grammar, D0LSystemAlgorithm& algo,
           TurtleBuilder2D& turtle, Mesh& mesh, OpenGLRenderer& renderer);

  void draw();

  glm::vec4 lineColor() const { return m_lineColor; }

 private:
  void drawControlPanel(float& nextY);
  void drawGrammarPanel(float& nextY);
  void drawSettingsPanel(float& nextY);
  void rebuildMesh();
  void applyGrammar();

  LSystemGrammar&     m_grammar;
  D0LSystemAlgorithm& m_algo;
  TurtleBuilder2D&    m_turtle;
  Mesh&               m_mesh;
  OpenGLRenderer&     m_renderer;

  float m_angleOverride;
  float m_stepLen;
  float m_zoom  = 1.f;
  float m_panX  = 0.f;
  float m_panY  = 0.f;

  glm::vec4 m_lineColor = {0.6f, 0.9f, 0.5f, 1.f};
  glm::vec4 m_bgColor   = {0.08f, 0.08f, 0.08f, 1.f};

  struct RuleEdit {
    char predecessor[2]  = {};
    char successor[256]  = {};
  };

  char                  m_axiomBuf[256] = {};
  std::vector<RuleEdit> m_ruleEdits;
};

}  // namespace D
