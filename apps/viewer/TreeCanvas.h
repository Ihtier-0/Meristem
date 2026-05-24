#pragma once

#include <memory>
#include <vector>
#include <string>

#include <QColor>
#include <QOpenGLWidget>

#include "algorithm/IPlantAlgorithm.h"
#include "algorithm/D0LSystemAlgorithm.h"
#include "algorithm/StochasticLSystemAlgorithm.h"
#include "algorithm/ParametricLSystemAlgorithm.h"
#include "geometry/TurtleBuilder2D.h"
#include "geometry/Mesh.h"
#include "grammar/LSystemGrammar.h"
#include "renderer/OpenGLRenderer.h"

namespace D {

class TreeCanvas : public QOpenGLWidget {
  Q_OBJECT

 public:
  enum class AlgoType {
    D0L = 0,
    Stochastic,
    ContextSensitive,
    ContextSensitive2L,
    Parametric,
    ContextSensitiveFlower
  };

  struct RuleEdit {
    char  predecessor[2]{};
    char  leftContext[2]{};
    char  rightContext[2]{};
    char  successor[256]{};
    float probability = 1.f;
  };

  struct ParametricEdit {
    char predecessor[2]{};
    char paramNames[32]{};
    char successorExpr[256]{};
  };

  struct ParamDef {
    char  name[16]{};
    float value = 0.f;
  };

  explicit TreeCanvas(QWidget* parent = nullptr);

  // State accessors — ControlPanel reads these at startup
  AlgoType algoType()    const { return m_algoType; }
  int      generation()  const;
  int      symbolCount() const;
  double   angle()       const { return m_angleOverride; }
  double   stepLen()     const { return m_stepLen; }
  double   zoom()        const { return m_zoom; }
  double   panX()        const { return m_panX; }
  double   panY()        const { return m_panY; }
  int      seed()        const { return m_seed; }
  double   flowerRadius() const { return m_flowerRadius; }
  QColor   lineColor()   const;
  QColor   flowerColor() const;
  QColor   bgColor()     const;

  const char*                        axiomBuf()       const { return m_axiomBuf; }
  const char*                        paramAxiomBuf()  const { return m_paramAxiomBuf; }
  const std::vector<RuleEdit>&       ruleEdits()      const { return m_ruleEdits; }
  const std::vector<ParametricEdit>& paramRuleEdits() const { return m_paramRuleEdits; }
  const std::vector<ParamDef>&       paramDefs()      const { return m_paramDefs; }

 public slots:
  void stepGeneration();
  void resetGeneration();
  void switchAlgo(int typeInt);

  void setAngle(double deg);
  void setStepLen(double len);
  void setZoom(double z);
  void setPanX(double x);
  void setPanY(double y);
  void setSeed(int seed);

  void setLineColor(QColor c);
  void setBgColor(QColor c);
  void setFlowerColor(QColor c);
  void setFlowerRadius(double r);

  void applyGrammar(const std::string& axiom, const std::vector<RuleEdit>& rules);
  void applyParametricGrammar(const std::string& axiom,
                               const std::vector<ParametricEdit>& rules,
                               const std::vector<ParamDef>& params);

 signals:
  void stateChanged(int generation, int symbols);
  void algoSwitched(int typeInt);

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

 private:
  void initLSystem();
  void rebuildMesh();
  void populateGrammarBuffers();
  static RuleEdit    ruleToEdit(const Rule& rule);
  static std::string wordToString(const Word& word);
  static std::string wordToParametricString(const Word& word);
  static Word        stringToWord(std::string_view s);
  static QColor toQColor(Vec4 c);
  static Vec4   toGlm(QColor c);

  std::unique_ptr<OpenGLRenderer> m_renderer;

  AlgoType                         m_algoType = AlgoType::D0L;
  std::unique_ptr<IPlantAlgorithm> m_algo;
  LSystemGrammar                   m_grammar;
  int                              m_seed = 42;

  TurtleBuilder2D m_turtle;
  Mesh            m_mesh;
  Mesh            m_flowerMesh;

  float m_angleOverride = 25.f;
  float m_stepLen       = 1.f;
  float m_zoom          = 1.f;
  float m_panX          = 0.f;
  float m_panY          = 0.f;
  float m_flowerRadius  = 0.3f;

  Vec4 m_lineColor   = {0.6f, 0.9f, 0.5f, 1.f};
  Vec4 m_flowerColor = {1.0f, 0.0f, 0.0f, 1.f};
  Vec4 m_bgColor     = {0.08f, 0.08f, 0.08f, 1.f};

  char                        m_axiomBuf[256]{};
  std::vector<RuleEdit>       m_ruleEdits;

  char                        m_paramAxiomBuf[256]{};
  std::vector<ParametricEdit> m_paramRuleEdits;
  std::vector<ParamDef>       m_paramDefs;
};

}  // namespace D
