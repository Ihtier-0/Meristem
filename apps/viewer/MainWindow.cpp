#include "MainWindow.h"

#include <QDockWidget>

#include "ControlPanel.h"
#include "TreeCanvas.h"

namespace D {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle("Meristem v0.1");
  resize(1280, 720);

  m_canvas = new TreeCanvas(this);
  m_canvas->setMinimumSize(640, 480);
  setCentralWidget(m_canvas);

  m_panel = new ControlPanel(m_canvas, this);

  auto* dock = new QDockWidget("Controls", this);
  dock->setWidget(m_panel);
  dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dock);
}

}  // namespace D
