#pragma once

#include <QMainWindow>

namespace D {

class TreeCanvas;
class ControlPanel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  TreeCanvas*   m_canvas = nullptr;
  ControlPanel* m_panel  = nullptr;
};

}  // namespace D
