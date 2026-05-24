#include "ControlPanel.h"

#include <algorithm>
#include <functional>

#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace D {

// ── Construction ──────────────────────────────────────────────────────────────

ControlPanel::ControlPanel(TreeCanvas* canvas, QWidget* parent)
    : QScrollArea(parent), m_canvas(canvas) {
  buildUi();
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setMinimumWidth(360);
  setMaximumWidth(380);

  connect(canvas, &TreeCanvas::stateChanged, this, &ControlPanel::onStateChanged);
  connect(canvas, &TreeCanvas::algoSwitched, this, &ControlPanel::onAlgoSwitched);
}

// ── UI construction ───────────────────────────────────────────────────────────

void ControlPanel::buildUi() {
  auto* root    = new QWidget;
  auto* rootLay = new QVBoxLayout(root);
  rootLay->setSpacing(6);
  rootLay->setContentsMargins(6, 6, 6, 6);

  rootLay->addWidget(buildControlSection());
  rootLay->addWidget(buildGrammarSection());
  rootLay->addWidget(buildSettingsSection());
  rootLay->addStretch();

  setWidget(root);
}

QWidget* ControlPanel::buildControlSection() {
  auto* box = new QGroupBox("L-System");
  auto* lay = new QVBoxLayout(box);
  lay->setSpacing(4);

  // Algo combo
  m_algoCombo = new QComboBox;
  m_algoCombo->addItems({
      "D0L (deterministic)", "Stochastic",
      "Context-sensitive (1L)", "Context-sensitive (2L)",
      "Parametric", "Context-sensitive (flower K)"});
  m_algoCombo->setCurrentIndex(static_cast<int>(m_canvas->algoType()));
  lay->addWidget(m_algoCombo);

  // Info label
  m_infoLabel = new QLabel;
  onStateChanged(m_canvas->generation(), m_canvas->symbolCount());
  lay->addWidget(m_infoLabel);

  lay->addSpacing(4);

  // Seed row (only for stochastic)
  m_seedRow = new QWidget;
  auto* seedLay = new QHBoxLayout(m_seedRow);
  seedLay->setContentsMargins(0, 0, 0, 0);
  seedLay->addWidget(new QLabel("Seed:"));
  m_seedSpin = new QSpinBox;
  m_seedSpin->setRange(0, 999999);
  m_seedSpin->setValue(m_canvas->seed());
  seedLay->addWidget(m_seedSpin);
  lay->addWidget(m_seedRow);

  // Numeric sliders
  auto makeSpinRow = [&](const QString& label, double val,
                         double minV, double maxV, double step,
                         int decimals) -> QDoubleSpinBox* {
    auto* row  = new QWidget;
    auto* hlay = new QHBoxLayout(row);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->addWidget(new QLabel(label));
    auto* spin = new QDoubleSpinBox;
    spin->setRange(minV, maxV);
    spin->setSingleStep(step);
    spin->setDecimals(decimals);
    spin->setValue(val);
    hlay->addWidget(spin);
    lay->addWidget(row);
    return spin;
  };

  m_angleSpin = makeSpinRow("Angle:",   m_canvas->angle(),   5.0,  90.0, 1.0,  1);
  m_stepSpin  = makeSpinRow("Step:",    m_canvas->stepLen(), 0.1,  10.0, 0.1,  2);
  m_zoomSpin  = makeSpinRow("Zoom:",    m_canvas->zoom(),    0.1,  20.0, 0.1,  2);
  m_panXSpin  = makeSpinRow("Pan X:",   m_canvas->panX(),  -100., 100., 0.5,  1);
  m_panYSpin  = makeSpinRow("Pan Y:",   m_canvas->panY(),  -100., 100., 0.5,  1);

  // Step / Reset buttons
  auto* btnRow = new QWidget;
  auto* btnLay = new QHBoxLayout(btnRow);
  btnLay->setContentsMargins(0, 0, 0, 0);
  auto* stepBtn  = new QPushButton("Step");
  auto* resetBtn = new QPushButton("Reset");
  btnLay->addWidget(stepBtn);
  btnLay->addWidget(resetBtn);
  lay->addWidget(btnRow);

  // ── Connections ──────────────────────────────────────────────────────────────

  connect(m_algoCombo, &QComboBox::currentIndexChanged,
          m_canvas, &TreeCanvas::switchAlgo);

  connect(m_seedSpin, &QSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setSeed);

  connect(m_angleSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setAngle);
  connect(m_stepSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setStepLen);
  connect(m_zoomSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setZoom);
  connect(m_panXSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setPanX);
  connect(m_panYSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setPanY);

  connect(stepBtn,  &QPushButton::clicked, m_canvas, &TreeCanvas::stepGeneration);
  connect(resetBtn, &QPushButton::clicked, m_canvas, &TreeCanvas::resetGeneration);

  // Update angle spin when canvas reports algo switched (parametric resets angle)
  connect(m_canvas, &TreeCanvas::algoSwitched, this, [this](int) {
    QSignalBlocker b(m_angleSpin);
    m_angleSpin->setValue(m_canvas->angle());
  });

  updateContextVisibility();
  return box;
}

QWidget* ControlPanel::buildGrammarSection() {
  auto* box = new QGroupBox("Grammar");
  auto* lay = new QVBoxLayout(box);

  m_grammarStack = new QStackedWidget;

  // ── Page 0: normal grammar ────────────────────────────────────────────────
  auto* normalPage = new QWidget;
  auto* nLay       = new QVBoxLayout(normalPage);
  nLay->setContentsMargins(0, 0, 0, 0);

  auto* axiomRow = new QWidget;
  auto* axHLay   = new QHBoxLayout(axiomRow);
  axHLay->setContentsMargins(0, 0, 0, 0);
  axHLay->addWidget(new QLabel("Axiom:"));
  m_axiomEdit = new QLineEdit(m_canvas->axiomBuf());
  axHLay->addWidget(m_axiomEdit);
  nLay->addWidget(axiomRow);

  m_rulesWidget = new QWidget;
  m_rulesLayout = new QVBoxLayout(m_rulesWidget);
  m_rulesLayout->setContentsMargins(0, 0, 0, 0);
  m_rulesLayout->setSpacing(2);
  nLay->addWidget(m_rulesWidget);

  auto* addRuleBtn = new QPushButton("+ Rule");
  nLay->addWidget(addRuleBtn);
  auto* applyBtn = new QPushButton("Apply");
  nLay->addWidget(applyBtn);

  connect(addRuleBtn, &QPushButton::clicked, this, &ControlPanel::onAddRuleClicked);
  connect(applyBtn,   &QPushButton::clicked, this, &ControlPanel::onApplyClicked);

  m_grammarStack->addWidget(normalPage);

  // ── Page 1: parametric grammar ────────────────────────────────────────────
  auto* paramPage = new QWidget;
  auto* pLay      = new QVBoxLayout(paramPage);
  pLay->setContentsMargins(0, 0, 0, 0);

  auto* paramAxRow = new QWidget;
  auto* paHLay     = new QHBoxLayout(paramAxRow);
  paHLay->setContentsMargins(0, 0, 0, 0);
  paHLay->addWidget(new QLabel("Axiom:"));
  m_paramAxiomEdit = new QLineEdit(m_canvas->paramAxiomBuf());
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
  auto* addDefBtn = new QPushButton("+ Param");
  pLay->addWidget(addDefBtn);

  auto* applyPBtn = new QPushButton("Apply");
  pLay->addWidget(applyPBtn);

  connect(addPRuleBtn, &QPushButton::clicked, this, &ControlPanel::onAddParamRuleClicked);
  connect(addDefBtn,   &QPushButton::clicked, this, &ControlPanel::onAddParamDefClicked);
  connect(applyPBtn,   &QPushButton::clicked, this, &ControlPanel::onApplyClicked);

  m_grammarStack->addWidget(paramPage);

  lay->addWidget(m_grammarStack);

  rebuildRuleRows();

  bool isParam = (m_canvas->algoType() == TreeCanvas::AlgoType::Parametric);
  m_grammarStack->setCurrentIndex(isParam ? 1 : 0);

  return box;
}

QWidget* ControlPanel::buildSettingsSection() {
  auto* box  = new QGroupBox("Settings");
  auto* form = new QFormLayout(box);

  form->addRow("Line color:",
      makeColorButton(m_canvas->lineColor(),
          [this](QColor c) { m_canvas->setLineColor(c); }));

  form->addRow("Background:",
      makeColorButton(m_canvas->bgColor(),
          [this](QColor c) { m_canvas->setBgColor(c); }));

  form->addRow("Flower color:",
      makeColorButton(m_canvas->flowerColor(),
          [this](QColor c) { m_canvas->setFlowerColor(c); }));

  auto* radSpin = new QDoubleSpinBox;
  radSpin->setRange(0.05, 3.0);
  radSpin->setSingleStep(0.05);
  radSpin->setDecimals(2);
  radSpin->setValue(m_canvas->flowerRadius());
  connect(radSpin, &QDoubleSpinBox::valueChanged,
          m_canvas, &TreeCanvas::setFlowerRadius);
  form->addRow("Flower radius:", radSpin);

  return box;
}

// ── Color button helper ───────────────────────────────────────────────────────

QPushButton* ControlPanel::makeColorButton(QColor initial,
                                            std::function<void(QColor)> onChange) {
  auto* btn = new QPushButton;
  btn->setMinimumWidth(60);
  btn->setFixedHeight(22);

  auto applyStyle = [btn](QColor c) {
    btn->setStyleSheet(
        QString("background-color: rgb(%1,%2,%3); border: 1px solid #555;")
            .arg(c.red()).arg(c.green()).arg(c.blue()));
  };
  applyStyle(initial);

  connect(btn, &QPushButton::clicked, this, [btn, onChange, applyStyle]() {
    QColor c = QColorDialog::getColor(
        btn->palette().button().color(), btn, "Pick colour");
    if (c.isValid()) {
      applyStyle(c);
      onChange(c);
    }
  });

  return btn;
}

// ── Rule rows — normal grammar ────────────────────────────────────────────────

void ControlPanel::rebuildRuleRows() {
  // Clear existing rows
  for (auto& row : m_normalRows)
    delete row.widget;
  m_normalRows.clear();

  for (const auto& re : m_canvas->ruleEdits())
    addNormalRuleRow(re);
}

void ControlPanel::addNormalRuleRow(const TreeCanvas::RuleEdit& re) {
  NormalRuleRow row;
  row.widget = new QWidget;
  auto* hlay = new QHBoxLayout(row.widget);
  hlay->setContentsMargins(0, 0, 0, 0);
  hlay->setSpacing(2);

  bool isContext = (m_canvas->algoType() == TreeCanvas::AlgoType::ContextSensitive ||
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
    if (it != m_normalRows.end()) {
      delete it->widget;
      m_normalRows.erase(it);
    }
  });
}

// ── Rule rows — parametric grammar ───────────────────────────────────────────

void ControlPanel::rebuildParamRuleRows() {
  for (auto& row : m_paramRows) delete row.widget;
  m_paramRows.clear();

  for (const auto& pe : m_canvas->paramRuleEdits())
    addParamRuleRow(pe);
}

void ControlPanel::addParamRuleRow(const TreeCanvas::ParametricEdit& pe) {
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
    if (it != m_paramRows.end()) {
      delete it->widget;
      m_paramRows.erase(it);
    }
  });
}

// ── Param def rows ────────────────────────────────────────────────────────────

void ControlPanel::rebuildParamDefRows() {
  for (auto& row : m_defRows) delete row.widget;
  m_defRows.clear();

  for (const auto& pd : m_canvas->paramDefs())
    addParamDefRow(pd);
}

void ControlPanel::addParamDefRow(const TreeCanvas::ParamDef& pd) {
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

  // Live param update → propagate to canvas immediately
  connect(row.value, &QDoubleSpinBox::valueChanged, this, [this](double) {
    onApplyClicked();
  });

  m_paramDefsLayout->addWidget(row.widget);
  QWidget* w = row.widget;
  m_defRows.push_back(row);

  connect(delBtn, &QPushButton::clicked, this, [this, w]() {
    auto it = std::find_if(m_defRows.begin(), m_defRows.end(),
        [w](const ParamDefRow& r) { return r.widget == w; });
    if (it != m_defRows.end()) {
      delete it->widget;
      m_defRows.erase(it);
    }
  });
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void ControlPanel::onStateChanged(int gen, int syms) {
  m_infoLabel->setText(
      QString("Generation: %1    Symbols: %2").arg(gen).arg(syms));
}

void ControlPanel::onAlgoSwitched(int typeInt) {
  auto type    = static_cast<TreeCanvas::AlgoType>(typeInt);
  bool isParam = (type == TreeCanvas::AlgoType::Parametric);

  m_grammarStack->setCurrentIndex(isParam ? 1 : 0);

  if (isParam) {
    {
      QSignalBlocker b(m_paramAxiomEdit);
      m_paramAxiomEdit->setText(m_canvas->paramAxiomBuf());
    }
    rebuildParamRuleRows();
    rebuildParamDefRows();
  } else {
    {
      QSignalBlocker b(m_axiomEdit);
      m_axiomEdit->setText(m_canvas->axiomBuf());
    }
    rebuildRuleRows();
  }

  updateContextVisibility();
}

void ControlPanel::updateContextVisibility() {
  auto type = m_canvas->algoType();
  m_seedRow->setVisible(type == TreeCanvas::AlgoType::Stochastic);
}

void ControlPanel::onApplyClicked() {
  bool isParam = (m_canvas->algoType() == TreeCanvas::AlgoType::Parametric);

  if (isParam) {
    std::vector<TreeCanvas::ParametricEdit> rules;
    for (const auto& row : m_paramRows) {
      TreeCanvas::ParametricEdit pe;
      auto pred   = row.pred->text().toStdString();
      auto params = row.params->text().toStdString();
      auto expr   = row.expr->text().toStdString();
      if (!pred.empty()) pe.predecessor[0] = pred[0];
      auto plen = std::min(params.size(), sizeof(pe.paramNames) - 1);
      std::copy_n(params.begin(), plen, pe.paramNames);
      auto elen = std::min(expr.size(), sizeof(pe.successorExpr) - 1);
      std::copy_n(expr.begin(), elen, pe.successorExpr);
      rules.push_back(pe);
    }
    std::vector<TreeCanvas::ParamDef> defs;
    for (const auto& row : m_defRows) {
      TreeCanvas::ParamDef pd;
      auto nm = row.name->text().toStdString();
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

void ControlPanel::onAddRuleClicked() {
  addNormalRuleRow({});
}

void ControlPanel::onAddParamRuleClicked() {
  addParamRuleRow({});
}

void ControlPanel::onAddParamDefClicked() {
  addParamDefRow({});
}

}  // namespace D
