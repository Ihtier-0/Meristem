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

  // Re-sync the algorithm combo to the canvas's current document state
  // (used to revert the selection when a preset switch is cancelled).
  void resyncCombo();

 signals:
  // Emitted when the user picks a preset in the combo. Routed to MainWindow so
  // it can prompt to save a modified document before switching.
  void presetRequested(int index);

 private:
  void updateSeedVisibility();

  // Index of the trailing "Custom" item in m_algoCombo (after the 6 presets).
  static constexpr int kCustomIndex = 6;

  TreeCanvas* m_canvas = nullptr;
  QComboBox* m_algoCombo = nullptr;
  QWidget* m_seedRow = nullptr;
  QSpinBox* m_seedSpin = nullptr;
  QDoubleSpinBox* m_angleSpin = nullptr;
  QDoubleSpinBox* m_stepSpin = nullptr;
};

}  // namespace D
