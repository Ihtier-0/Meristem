#pragma once

#include <QDialog>

namespace D {

class TreeCanvas;

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(TreeCanvas* canvas, QWidget* parent = nullptr);
};

}  // namespace D
