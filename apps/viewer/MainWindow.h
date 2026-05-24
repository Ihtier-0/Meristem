#pragma once

#include <QMainWindow>

class QLabel;

namespace D {

class TreeCanvas;
class ControlPanel;

class MainWindow final : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

 private:
  void createMenus();
  void refreshStatus();

  TreeCanvas* m_canvas = nullptr;
  ControlPanel* m_panel = nullptr;
  QLabel* m_statusLabel = nullptr;
};

}  // namespace D
