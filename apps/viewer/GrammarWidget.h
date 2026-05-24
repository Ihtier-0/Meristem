#pragma once

#include <vector>

#include <QGroupBox>

#include "TreeCanvas.h"

class QDoubleSpinBox;
class QLineEdit;
class QStackedWidget;
class QVBoxLayout;

namespace D {

class GrammarWidget : public QGroupBox {
  Q_OBJECT

 public:
  explicit GrammarWidget(TreeCanvas* canvas, QWidget* parent = nullptr);

 public slots:
  void onAlgoSwitched(int typeInt);

 private slots:
  void onApplyClicked();
  void onAddRuleClicked();
  void onAddParamRuleClicked();
  void onAddParamDefClicked();

 private:
  void rebuildRuleRows();
  void rebuildParamRuleRows();
  void rebuildParamDefRows();
  void addNormalRuleRow(const TreeCanvas::RuleEdit& re);
  void addParamRuleRow(const TreeCanvas::ParametricEdit& pe);
  void addParamDefRow(const TreeCanvas::ParamDef& pd);

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

  TreeCanvas*     m_canvas           = nullptr;
  QStackedWidget* m_grammarStack     = nullptr;
  QLineEdit*      m_axiomEdit        = nullptr;
  QWidget*        m_rulesWidget      = nullptr;
  QVBoxLayout*    m_rulesLayout      = nullptr;
  QLineEdit*      m_paramAxiomEdit   = nullptr;
  QWidget*        m_paramRulesWidget = nullptr;
  QVBoxLayout*    m_paramRulesLayout = nullptr;
  QWidget*        m_paramDefsWidget  = nullptr;
  QVBoxLayout*    m_paramDefsLayout  = nullptr;

  std::vector<NormalRuleRow> m_normalRows;
  std::vector<ParamRuleRow>  m_paramRows;
  std::vector<ParamDefRow>   m_defRows;
};

}  // namespace D
