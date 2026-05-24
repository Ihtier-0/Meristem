#pragma once

#include <vector>

#include <QColor>
#include <QScrollArea>
#include <QWidget>

#include "TreeCanvas.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QStackedWidget;
class QVBoxLayout;
class QLineEdit;

namespace D {

class ControlPanel : public QScrollArea {
  Q_OBJECT

 public:
  explicit ControlPanel(TreeCanvas* canvas, QWidget* parent = nullptr);

 private slots:
  void onStateChanged(int gen, int syms);
  void onAlgoSwitched(int typeInt);

  void onApplyClicked();
  void onAddRuleClicked();
  void onAddParamRuleClicked();
  void onAddParamDefClicked();

 private:
  void buildUi();

  QWidget* buildControlSection();
  QWidget* buildGrammarSection();

  void rebuildRuleRows();
  void rebuildParamRuleRows();
  void rebuildParamDefRows();

  void addNormalRuleRow(const TreeCanvas::RuleEdit& re);
  void addParamRuleRow(const TreeCanvas::ParametricEdit& pe);
  void addParamDefRow(const TreeCanvas::ParamDef& pd);

  void updateContextVisibility();

  TreeCanvas* m_canvas;

  // Control section
  QComboBox*      m_algoCombo    = nullptr;
  QLabel*         m_infoLabel    = nullptr;
  QSpinBox*       m_seedSpin     = nullptr;
  QWidget*        m_seedRow      = nullptr;
  QDoubleSpinBox* m_angleSpin    = nullptr;
  QDoubleSpinBox* m_stepSpin     = nullptr;
  QDoubleSpinBox* m_zoomSpin     = nullptr;
  QDoubleSpinBox* m_panXSpin     = nullptr;
  QDoubleSpinBox* m_panYSpin     = nullptr;

  // Grammar section — stacked (0=normal, 1=parametric)
  QStackedWidget* m_grammarStack  = nullptr;

  // Normal grammar page
  QLineEdit*   m_axiomEdit     = nullptr;
  QWidget*     m_rulesWidget   = nullptr;
  QVBoxLayout* m_rulesLayout   = nullptr;

  // Parametric grammar page
  QLineEdit*   m_paramAxiomEdit   = nullptr;
  QWidget*     m_paramRulesWidget = nullptr;
  QVBoxLayout* m_paramRulesLayout = nullptr;
  QWidget*     m_paramDefsWidget  = nullptr;
  QVBoxLayout* m_paramDefsLayout  = nullptr;

  struct NormalRuleRow {
    QWidget*        widget   = nullptr;
    QLineEdit*      leftCtx  = nullptr;
    QLineEdit*      pred     = nullptr;
    QLineEdit*      rightCtx = nullptr;
    QLineEdit*      succ     = nullptr;
    QDoubleSpinBox* prob     = nullptr;
  };

  struct ParamRuleRow {
    QWidget*   widget = nullptr;
    QLineEdit* pred   = nullptr;
    QLineEdit* params = nullptr;
    QLineEdit* expr   = nullptr;
  };

  struct ParamDefRow {
    QWidget*        widget = nullptr;
    QLineEdit*      name   = nullptr;
    QDoubleSpinBox* value  = nullptr;
  };

  std::vector<NormalRuleRow> m_normalRows;
  std::vector<ParamRuleRow>  m_paramRows;
  std::vector<ParamDefRow>   m_defRows;
};

}  // namespace D
