#pragma once

#include <QMainWindow>

class QLabel;

namespace D {

class TreeCanvas;
class ControlPanel;

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr);

 private:
  void createMenus();
  void refreshStatus();

  TreeCanvas*   m_canvas      = nullptr;
  ControlPanel* m_panel       = nullptr;
  QLabel*       m_statusLabel = nullptr;
};

}  // namespace D
