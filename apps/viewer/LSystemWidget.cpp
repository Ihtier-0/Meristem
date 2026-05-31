#include "LSystemWidget.h"

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace D {

LSystemWidget::LSystemWidget(TreeCanvas* canvas, QWidget* parent)
    : QGroupBox("L-System", parent), m_canvas(canvas) {
  auto* lay = new QVBoxLayout(this);
  lay->setSpacing(4);

  m_algoCombo = new QComboBox;
  m_algoCombo->addItems({"D0L (deterministic)", "Stochastic", "Context-sensitive (1L)",
                         "Context-sensitive (2L)", "Parametric", "Context-sensitive (flower K)",
                         "Context-sensitive (flower, article)"});
  m_algoCombo->setCurrentIndex(static_cast<int>(canvas->algoType()));
  lay->addWidget(m_algoCombo);

  m_seedRow = new QWidget;
  auto* seedLay = new QHBoxLayout(m_seedRow);
  seedLay->setContentsMargins(0, 0, 0, 0);
  seedLay->addWidget(new QLabel("Seed:"));
  m_seedSpin = new QSpinBox;
  m_seedSpin->setRange(0, 999999);
  m_seedSpin->setValue(canvas->seed());
  seedLay->addWidget(m_seedSpin);
  lay->addWidget(m_seedRow);

  auto makeSpinRow = [&](const QString& label, double val, double minV, double maxV, double step,
                         int decimals) -> QDoubleSpinBox* {
    auto* row = new QWidget;
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

  m_angleSpin = makeSpinRow("Angle:", canvas->angle(), 5.0, 90.0, 1.0, 1);
  m_stepSpin = makeSpinRow("Step:", canvas->stepLen(), 0.1, 10.0, 0.1, 2);

  auto* btnRow = new QWidget;
  auto* btnLay = new QHBoxLayout(btnRow);
  btnLay->setContentsMargins(0, 0, 0, 0);
  auto* stepBtn = new QPushButton("Step");
  auto* resetBtn = new QPushButton("Reset");
  btnLay->addWidget(stepBtn);
  btnLay->addWidget(resetBtn);
  lay->addWidget(btnRow);

  connect(m_algoCombo, &QComboBox::currentIndexChanged, canvas, &TreeCanvas::switchAlgo);
  connect(m_seedSpin, &QSpinBox::valueChanged, canvas, &TreeCanvas::setSeed);
  connect(m_angleSpin, &QDoubleSpinBox::valueChanged, canvas, &TreeCanvas::setAngle);
  connect(m_stepSpin, &QDoubleSpinBox::valueChanged, canvas, &TreeCanvas::setStepLen);
  connect(stepBtn, &QPushButton::clicked, canvas, &TreeCanvas::stepGeneration);
  connect(resetBtn, &QPushButton::clicked, canvas, &TreeCanvas::resetGeneration);

  connect(canvas, &TreeCanvas::algoSwitched, this, [this](int) {
    QSignalBlocker b(m_angleSpin);
    m_angleSpin->setValue(m_canvas->angle());
  });

  updateSeedVisibility();
}

void LSystemWidget::onAlgoSwitched(int) { updateSeedVisibility(); }

void LSystemWidget::updateSeedVisibility() {
  m_seedRow->setVisible(m_canvas->algoType() == TreeCanvas::AlgoType::Stochastic);
}

}  // namespace D
