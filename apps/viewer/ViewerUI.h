#pragma once

#include <string>
#include <vector>

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

 private:
  void drawControlPanel();
  void drawGrammarPanel();
  void rebuildMesh();
  void applyGrammar();

  LSystemGrammar&    m_grammar;
  D0LSystemAlgorithm& m_algo;
  TurtleBuilder2D&   m_turtle;
  Mesh&              m_mesh;
  OpenGLRenderer&    m_renderer;

  float m_angleOverride;
  float m_stepLen;
  float m_zoom  = 1.f;
  float m_panX  = 0.f;
  float m_panY  = 0.f;

  struct RuleEdit {
    char predecessor[2]  = {};
    char successor[256]  = {};
  };

  char                  m_axiomBuf[256] = {};
  std::vector<RuleEdit> m_ruleEdits;
};

}  // namespace D
