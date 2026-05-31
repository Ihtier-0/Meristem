#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QColor>
#include <QOpenGLWidget>
#include <QPoint>

#include "algorithm/D0LSystemAlgorithm.h"
#include "algorithm/IPlantAlgorithm.h"
#include "algorithm/ParametricLSystemAlgorithm.h"
#include "algorithm/StochasticLSystemAlgorithm.h"

#include "geometry/Mesh.h"
#include "geometry/TurtleBuilder2D.h"
#include "grammar/LSystemGrammar.h"

#include "renderer/OpenGLRenderer.h"

namespace D {

class TreeCanvas final : public QOpenGLWidget {
  Q_OBJECT

 public:
  enum class AlgoType {
    D0L = 0,
    Stochastic,
    ContextSensitive,
    ContextSensitive2L,
    Parametric,
    ContextFlower,
  };

  struct RuleEdit {
    char predecessor[2]{};
    char leftContext[8]{};
    char rightContext[8]{};
    char successor[256]{};
    float probability = 1.f;
  };

  struct ParametricEdit {
    char predecessor[2]{};
    char paramNames[32]{};
    char successorExpr[256]{};
  };

  struct ParamDef {
    char name[16]{};
    float value = 0.f;
  };

  struct ContextEdit {
    char ignore[32]{};    // symbols skipped during context search (e.g. "+-|")
    char push[2]{};       // branch-open symbol (single char or empty)
    char pop[2]{};        // branch-close symbol (single char or empty)
    bool includeSiblings = false;
    bool strictMode = false;
  };

  explicit TreeCanvas(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  // State accessors — ControlPanel reads these at startup
  AlgoType algoType() const { return m_algoType; }
  int generation() const;
  int symbolCount() const;
  double angle() const { return static_cast<double>(m_angle); }
  double stepLen() const { return static_cast<double>(m_stepLen); }
  double zoom() const { return m_zoom; }
  double panX() const { return m_panX; }
  double panY() const { return m_panY; }
  int seed() const { return m_seed; }
  double flowerRadius() const { return m_flowerRadius; }
  TurtleSymbols symbols() const { return m_turtle.symbols(); }
  QColor lineColor() const;
  QColor flowerColor() const;
  QColor bgColor() const;

  const char* axiomBuf() const { return m_axiomBuf; }
  const char* paramAxiomBuf() const { return m_paramAxiomBuf; }
  const std::vector<RuleEdit>& ruleEdits() const { return m_ruleEdits; }
  const std::vector<ParametricEdit>& paramRuleEdits() const { return m_paramRuleEdits; }
  const std::vector<ParamDef>& paramDefs() const { return m_paramDefs; }
  ContextEdit contextEdit() const;

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
  void setSymbols(TurtleSymbols s);

  void applyGrammar(const std::string& axiom, const std::vector<RuleEdit>& rules,
                    const ContextEdit& ctx);
  void applyParametricGrammar(const std::string& axiom, const std::vector<ParametricEdit>& rules,
                              const std::vector<ParamDef>& params);

 signals:
  void stateChanged(int generation, int symbols);
  void algoSwitched(int typeInt);
  void viewChanged(double zoom, double panX, double panY);

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  void mousePressEvent(QMouseEvent* e) override;
  void mouseMoveEvent(QMouseEvent* e) override;
  void mouseReleaseEvent(QMouseEvent* e) override;
  void wheelEvent(QWheelEvent* e) override;
  void keyPressEvent(QKeyEvent* e) override;

 private:
  void initLSystem();
  void rebuildMesh();
  void populateGrammarBuffers();

  std::unique_ptr<OpenGLRenderer> m_renderer;

  QPoint m_lastMousePos;
  bool m_panning = false;

  AlgoType m_algoType = AlgoType::D0L;
  std::unique_ptr<IPlantAlgorithm> m_algo;
  LSystemGrammar m_grammar;
  int m_seed = 42;

  TurtleBuilder2D m_turtle;
  Mesh m_mesh;
  Mesh m_flowerMesh;

  float m_angle   = 25.f;
  float m_stepLen = 1.f;

  float m_zoom = 1.f;
  float m_panX = 0.f;
  float m_panY = 0.f;
  float m_flowerRadius = 0.3f;

  Vec4 m_lineColor = {0.6f, 0.9f, 0.5f, 1.f};
  Vec4 m_flowerColor = {1.0f, 0.0f, 0.0f, 1.f};
  Vec4 m_bgColor = {0.08f, 0.08f, 0.08f, 1.f};

  char m_axiomBuf[256]{};
  std::vector<RuleEdit> m_ruleEdits;

  char m_paramAxiomBuf[256]{};
  std::vector<ParametricEdit> m_paramRuleEdits;
  std::vector<ParamDef> m_paramDefs;
};

}  // namespace D
