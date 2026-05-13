#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec4.hpp>

#include "algorithm/IPlantAlgorithm.h"
#include "algorithm/D0LSystemAlgorithm.h"
#include "algorithm/StochasticLSystemAlgorithm.h"
#include "geometry/TurtleBuilder2D.h"
#include "geometry/Mesh.h"
#include "renderer/OpenGLRenderer.h"

namespace D {

class ViewerUI {
 public:
  explicit ViewerUI(OpenGLRenderer& renderer);

  void draw();

  glm::vec4 lineColor() const { return m_lineColor; }
  const Mesh& mesh() const { return m_mesh; }

 private:
  enum class AlgoType { D0L = 0, Stochastic = 1 };

  void drawControlPanel(float& nextY);
  void drawGrammarPanel(float& nextY);
  void drawSettingsPanel(float& nextY);
  void rebuildMesh();
  void applyGrammar();
  void switchAlgo(AlgoType type);

  // Returns the grammar currently managed by the active algorithm
  LSystemGrammar& activeGrammar();

  OpenGLRenderer& m_renderer;

  AlgoType m_algoType = AlgoType::D0L;
  std::unique_ptr<IPlantAlgorithm> m_algo;

  // Owned grammar (edited by Grammar panel and reapplied)
  LSystemGrammar m_grammar;

  // Stochastic seed (only meaningful when m_algoType == Stochastic)
  int m_seed = 42;

  TurtleBuilder2D m_turtle;
  Mesh            m_mesh;

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
    float probability    = 1.f;
  };

  char                  m_axiomBuf[256] = {};
  std::vector<RuleEdit> m_ruleEdits;
};

}  // namespace D
