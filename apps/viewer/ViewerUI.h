#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <glm/vec4.hpp>

#include "algorithm/IPlantAlgorithm.h"
#include "algorithm/D0LSystemAlgorithm.h"
#include "algorithm/StochasticLSystemAlgorithm.h"
#include "algorithm/ContextSensitiveLSystemAlgorithm.h"
#include "algorithm/ParametricLSystemAlgorithm.h"
#include "geometry/TurtleBuilder2D.h"
#include "geometry/Mesh.h"
#include "renderer/OpenGLRenderer.h"

namespace D {

class ViewerUI {
 public:
  explicit ViewerUI(OpenGLRenderer& renderer);

  void draw();

  glm::vec4 lineColor()   const { return m_lineColor; }
  glm::vec4 flowerColor() const { return m_flowerColor; }
  const Mesh& mesh()        const { return m_mesh; }
  const Mesh& flowerMesh()  const { return m_flowerMesh; }

 private:
  enum class AlgoType { D0L = 0, Stochastic = 1, ContextSensitive = 2, ContextSensitive2L = 3, Parametric = 4, ContextSensitiveFlower = 5 };

  struct RuleEdit {
    char  predecessor[2] = {};
    char  leftContext[2] = {};
    char  rightContext[2]= {};
    char  successor[256] = {};
    float probability    = 1.f;
  };

  struct ParametricEdit {
    char predecessor[2]   = {};
    char paramNames[32]   = {};   // comma-separated, e.g. "s" or "s,t"
    char successorExpr[256] = {};
  };

  struct ParamDef {
    char  name[16] = {};
    float value    = 0.f;
  };

  void drawControlPanel(float& nextY);
  void drawGrammarPanel(float& nextY);
  void drawSettingsPanel(float& nextY);
  void rebuildMesh();
  void applyGrammar();
  void switchAlgo(AlgoType type);
  static RuleEdit ruleToEdit(const Rule& rule);
  void applyParametricGrammar();

  OpenGLRenderer& m_renderer;

  AlgoType m_algoType = AlgoType::D0L;
  std::unique_ptr<IPlantAlgorithm> m_algo;

  LSystemGrammar m_grammar;
  int            m_seed = 42;

  TurtleBuilder2D m_turtle;
  Mesh            m_mesh;

  float m_angleOverride;
  float m_stepLen;
  float m_zoom  = 1.f;
  float m_panX  = 0.f;
  float m_panY  = 0.f;

  glm::vec4 m_lineColor   = {0.6f, 0.9f, 0.5f, 1.f};
  glm::vec4 m_flowerColor = {1.0f, 0.0f, 0.0f, 1.f};
  glm::vec4 m_bgColor     = {0.08f, 0.08f, 0.08f, 1.f};
  float     m_flowerRadius = 0.3f;
  Mesh      m_flowerMesh;

  char                  m_axiomBuf[256] = {};
  std::vector<RuleEdit> m_ruleEdits;

  char                       m_paramAxiomBuf[256] = {};
  std::vector<ParametricEdit> m_paramRuleEdits;
  std::vector<ParamDef>      m_paramDefs;
};

}  // namespace D
