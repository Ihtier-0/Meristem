#pragma once

#include <QGroupBox>

#include "TreeCanvas.h"

class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QSpinBox;

namespace D {

class LSystemWidget final : public QGroupBox {
  Q_OBJECT

 public:
  explicit LSystemWidget(TreeCanvas* canvas, QWidget* parent = nullptr);

 public slots:
  void onAlgoSwitched(int typeInt);
  void onViewChanged(double zoom, double panX, double panY);

 private:
  void updateSeedVisibility();

  TreeCanvas* m_canvas = nullptr;
  QComboBox* m_algoCombo = nullptr;
  QWidget* m_seedRow = nullptr;
  QSpinBox* m_seedSpin = nullptr;
  QDoubleSpinBox* m_angleSpin = nullptr;
  QDoubleSpinBox* m_stepSpin = nullptr;
  QDoubleSpinBox* m_zoomSpin = nullptr;
  QDoubleSpinBox* m_panXSpin = nullptr;
  QDoubleSpinBox* m_panYSpin = nullptr;
};

}  // namespace D
