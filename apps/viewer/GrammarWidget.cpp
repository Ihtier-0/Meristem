#include "GrammarWidget.h"

#include <algorithm>

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace D {

GrammarWidget::GrammarWidget(TreeCanvas* canvas, QWidget* parent)
    : QGroupBox("Grammar", parent), m_canvas(canvas) {
  auto* lay = new QVBoxLayout(this);

  m_grammarStack = new QStackedWidget;

  // ── Page 0: normal grammar ────────────────────────────────────────────────────

  auto* normalPage = new QWidget;
  auto* nLay       = new QVBoxLayout(normalPage);
  nLay->setContentsMargins(0, 0, 0, 0);

  auto* axiomRow = new QWidget;
  auto* axHLay   = new QHBoxLayout(axiomRow);
  axHLay->setContentsMargins(0, 0, 0, 0);
  axHLay->addWidget(new QLabel("Axiom:"));
  m_axiomEdit = new QLineEdit(canvas->axiomBuf());
  axHLay->addWidget(m_axiomEdit);
  nLay->addWidget(axiomRow);

  m_rulesWidget = new QWidget;
  m_rulesLayout = new QVBoxLayout(m_rulesWidget);
  m_rulesLayout->setContentsMargins(0, 0, 0, 0);
  m_rulesLayout->setSpacing(2);
  nLay->addWidget(m_rulesWidget);

  auto* addRuleBtn = new QPushButton("+ Rule");
  auto* applyBtn   = new QPushButton("Apply");
  nLay->addWidget(addRuleBtn);
  nLay->addWidget(applyBtn);

  connect(addRuleBtn, &QPushButton::clicked, this, &GrammarWidget::onAddRuleClicked);
  connect(applyBtn,   &QPushButton::clicked, this, &GrammarWidget::onApplyClicked);

  m_grammarStack->addWidget(normalPage);

  // ── Page 1: parametric grammar ────────────────────────────────────────────────

  auto* paramPage = new QWidget;
  auto* pLay      = new QVBoxLayout(paramPage);
  pLay->setContentsMargins(0, 0, 0, 0);

  auto* paramAxRow = new QWidget;
  auto* paHLay     = new QHBoxLayout(paramAxRow);
  paHLay->setContentsMargins(0, 0, 0, 0);
  paHLay->addWidget(new QLabel("Axiom:"));
  m_paramAxiomEdit = new QLineEdit(canvas->paramAxiomBuf());
  paHLay->addWidget(m_paramAxiomEdit);
  pLay->addWidget(paramAxRow);

  pLay->addWidget(new QLabel("Rules  pred(params) → expr:"));
  m_paramRulesWidget = new QWidget;
  m_paramRulesLayout = new QVBoxLayout(m_paramRulesWidget);
  m_paramRulesLayout->setContentsMargins(0, 0, 0, 0);
  m_paramRulesLayout->setSpacing(2);
  pLay->addWidget(m_paramRulesWidget);

  auto* addPRuleBtn = new QPushButton("+ Rule");
  pLay->addWidget(addPRuleBtn);

  pLay->addWidget(new QLabel("Global params:"));
  m_paramDefsWidget = new QWidget;
  m_paramDefsLayout = new QVBoxLayout(m_paramDefsWidget);
  m_paramDefsLayout->setContentsMargins(0, 0, 0, 0);
  m_paramDefsLayout->setSpacing(2);
  pLay->addWidget(m_paramDefsWidget);

  auto* addDefBtn  = new QPushButton("+ Param");
  auto* applyPBtn  = new QPushButton("Apply");
  pLay->addWidget(addDefBtn);
  pLay->addWidget(applyPBtn);

  connect(addPRuleBtn, &QPushButton::clicked, this, &GrammarWidget::onAddParamRuleClicked);
  connect(addDefBtn,   &QPushButton::clicked, this, &GrammarWidget::onAddParamDefClicked);
  connect(applyPBtn,   &QPushButton::clicked, this, &GrammarWidget::onApplyClicked);

  m_grammarStack->addWidget(paramPage);
  lay->addWidget(m_grammarStack);

  rebuildRuleRows();

  bool isParam = (canvas->algoType() == TreeCanvas::AlgoType::Parametric);
  m_grammarStack->setCurrentIndex(isParam ? 1 : 0);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void GrammarWidget::onAlgoSwitched(int typeInt) {
  auto type    = static_cast<TreeCanvas::AlgoType>(typeInt);
  bool isParam = (type == TreeCanvas::AlgoType::Parametric);

  m_grammarStack->setCurrentIndex(isParam ? 1 : 0);

  if (isParam) {
    { QSignalBlocker b(m_paramAxiomEdit);
      m_paramAxiomEdit->setText(m_canvas->paramAxiomBuf()); }
    rebuildParamRuleRows();
    rebuildParamDefRows();
  } else {
    { QSignalBlocker b(m_axiomEdit);
      m_axiomEdit->setText(m_canvas->axiomBuf()); }
    rebuildRuleRows();
  }
}

void GrammarWidget::onApplyClicked() {
  bool isParam = (m_canvas->algoType() == TreeCanvas::AlgoType::Parametric);

  if (isParam) {
    std::vector<TreeCanvas::ParametricEdit> rules;
    for (const auto& row : m_paramRows) {
      TreeCanvas::ParametricEdit pe;
      auto pred   = row.pred->text().toStdString();
      auto params = row.params->text().toStdString();
      auto expr   = row.expr->text().toStdString();
      if (!pred.empty()) pe.predecessor[0] = pred[0];
      auto plen = std::min(params.size(), sizeof(pe.paramNames)   - 1);
      auto elen = std::min(expr.size(),   sizeof(pe.successorExpr) - 1);
      std::copy_n(params.begin(), plen, pe.paramNames);
      std::copy_n(expr.begin(),   elen, pe.successorExpr);
      rules.push_back(pe);
    }
    std::vector<TreeCanvas::ParamDef> defs;
    for (const auto& row : m_defRows) {
      TreeCanvas::ParamDef pd;
      auto nm   = row.name->text().toStdString();
      auto nlen = std::min(nm.size(), sizeof(pd.name) - 1);
      std::copy_n(nm.begin(), nlen, pd.name);
      pd.value = static_cast<float>(row.value->value());
      defs.push_back(pd);
    }
    m_canvas->applyParametricGrammar(
        m_paramAxiomEdit->text().toStdString(), rules, defs);
  } else {
    std::vector<TreeCanvas::RuleEdit> rules;
    for (const auto& row : m_normalRows) {
      TreeCanvas::RuleEdit re;
      auto pred = row.pred->text().toStdString();
      auto lc   = row.leftCtx->text().toStdString();
      auto rc   = row.rightCtx->text().toStdString();
      auto succ = row.succ->text().toStdString();
      if (!pred.empty()) re.predecessor[0] = pred[0];
      if (!lc.empty())   re.leftContext[0]  = lc[0];
      if (!rc.empty())   re.rightContext[0] = rc[0];
      auto slen = std::min(succ.size(), sizeof(re.successor) - 1);
      std::copy_n(succ.begin(), slen, re.successor);
      re.probability = static_cast<float>(row.prob->value());
      rules.push_back(re);
    }
    m_canvas->applyGrammar(m_axiomEdit->text().toStdString(), rules);
  }
}

void GrammarWidget::onAddRuleClicked()      { addNormalRuleRow({}); }
void GrammarWidget::onAddParamRuleClicked() { addParamRuleRow({}); }
void GrammarWidget::onAddParamDefClicked()  { addParamDefRow({}); }

// ── Row builders ──────────────────────────────────────────────────────────────

void GrammarWidget::rebuildRuleRows() {
  for (auto& row : m_normalRows) delete row.widget;
  m_normalRows.clear();
  for (const auto& re : m_canvas->ruleEdits())
    addNormalRuleRow(re);
}

void GrammarWidget::rebuildParamRuleRows() {
  for (auto& row : m_paramRows) delete row.widget;
  m_paramRows.clear();
  for (const auto& pe : m_canvas->paramRuleEdits())
    addParamRuleRow(pe);
}

void GrammarWidget::rebuildParamDefRows() {
  for (auto& row : m_defRows) delete row.widget;
  m_defRows.clear();
  for (const auto& pd : m_canvas->paramDefs())
    addParamDefRow(pd);
}

void GrammarWidget::addNormalRuleRow(const TreeCanvas::RuleEdit& re) {
  NormalRuleRow row;
  row.widget = new QWidget;
  auto* hlay = new QHBoxLayout(row.widget);
  hlay->setContentsMargins(0, 0, 0, 0);
  hlay->setSpacing(2);

  bool isContext = (m_canvas->algoType() == TreeCanvas::AlgoType::ContextSensitive  ||
                    m_canvas->algoType() == TreeCanvas::AlgoType::ContextSensitive2L ||
                    m_canvas->algoType() == TreeCanvas::AlgoType::ContextSensitiveFlower);
  bool isStoch   = (m_canvas->algoType() == TreeCanvas::AlgoType::Stochastic);

  row.leftCtx = new QLineEdit(re.leftContext[0] ? QString(re.leftContext[0]) : "");
  row.leftCtx->setPlaceholderText("lc");
  row.leftCtx->setFixedWidth(24);
  row.leftCtx->setVisible(isContext);
  hlay->addWidget(row.leftCtx);
  if (isContext) hlay->addWidget(new QLabel("<"));

  row.pred = new QLineEdit(re.predecessor[0] ? QString(re.predecessor[0]) : "");
  row.pred->setFixedWidth(20);
  hlay->addWidget(row.pred);

  row.rightCtx = new QLineEdit(re.rightContext[0] ? QString(re.rightContext[0]) : "");
  row.rightCtx->setPlaceholderText("rc");
  row.rightCtx->setFixedWidth(24);
  row.rightCtx->setVisible(isContext);
  if (isContext) {
    hlay->addWidget(new QLabel(">"));
    hlay->addWidget(row.rightCtx);
  }

  hlay->addWidget(new QLabel("→"));
  row.succ = new QLineEdit(re.successor);
  hlay->addWidget(row.succ);

  row.prob = new QDoubleSpinBox;
  row.prob->setRange(0.0, 1.0);
  row.prob->setSingleStep(0.05);
  row.prob->setDecimals(2);
  row.prob->setValue(static_cast<double>(re.probability));
  row.prob->setFixedWidth(56);
  row.prob->setVisible(isStoch);
  hlay->addWidget(row.prob);

  auto* delBtn = new QPushButton("✕");
  delBtn->setFixedWidth(24);
  hlay->addWidget(delBtn);

  m_rulesLayout->addWidget(row.widget);
  QWidget* w = row.widget;
  m_normalRows.push_back(row);

  connect(delBtn, &QPushButton::clicked, this, [this, w]() {
    auto it = std::find_if(m_normalRows.begin(), m_normalRows.end(),
        [w](const NormalRuleRow& r) { return r.widget == w; });
    if (it != m_normalRows.end()) { delete it->widget; m_normalRows.erase(it); }
  });
}

void GrammarWidget::addParamRuleRow(const TreeCanvas::ParametricEdit& pe) {
  ParamRuleRow row;
  row.widget = new QWidget;
  auto* hlay = new QHBoxLayout(row.widget);
  hlay->setContentsMargins(0, 0, 0, 0);
  hlay->setSpacing(2);

  row.pred = new QLineEdit(pe.predecessor[0] ? QString(pe.predecessor[0]) : "");
  row.pred->setFixedWidth(18);
  hlay->addWidget(row.pred);
  hlay->addWidget(new QLabel("("));
  row.params = new QLineEdit(pe.paramNames);
  row.params->setFixedWidth(40);
  hlay->addWidget(row.params);
  hlay->addWidget(new QLabel(")→"));
  row.expr = new QLineEdit(pe.successorExpr);
  hlay->addWidget(row.expr);

  auto* delBtn = new QPushButton("✕");
  delBtn->setFixedWidth(24);
  hlay->addWidget(delBtn);

  m_paramRulesLayout->addWidget(row.widget);
  QWidget* w = row.widget;
  m_paramRows.push_back(row);

  connect(delBtn, &QPushButton::clicked, this, [this, w]() {
    auto it = std::find_if(m_paramRows.begin(), m_paramRows.end(),
        [w](const ParamRuleRow& r) { return r.widget == w; });
    if (it != m_paramRows.end()) { delete it->widget; m_paramRows.erase(it); }
  });
}

void GrammarWidget::addParamDefRow(const TreeCanvas::ParamDef& pd) {
  ParamDefRow row;
  row.widget = new QWidget;
  auto* hlay = new QHBoxLayout(row.widget);
  hlay->setContentsMargins(0, 0, 0, 0);
  hlay->setSpacing(2);

  row.name = new QLineEdit(pd.name);
  row.name->setFixedWidth(50);
  hlay->addWidget(row.name);
  hlay->addWidget(new QLabel("="));
  row.value = new QDoubleSpinBox;
  row.value->setRange(0.01, 10.0);
  row.value->setSingleStep(0.01);
  row.value->setDecimals(3);
  row.value->setValue(static_cast<double>(pd.value));
  hlay->addWidget(row.value);

  auto* delBtn = new QPushButton("✕");
  delBtn->setFixedWidth(24);
  hlay->addWidget(delBtn);

  connect(row.value, &QDoubleSpinBox::valueChanged, this, [this](double) {
    onApplyClicked();
  });

  m_paramDefsLayout->addWidget(row.widget);
  QWidget* w = row.widget;
  m_defRows.push_back(row);

  connect(delBtn, &QPushButton::clicked, this, [this, w]() {
    auto it = std::find_if(m_defRows.begin(), m_defRows.end(),
        [w](const ParamDefRow& r) { return r.widget == w; });
    if (it != m_defRows.end()) { delete it->widget; m_defRows.erase(it); }
  });
}

}  // namespace D
