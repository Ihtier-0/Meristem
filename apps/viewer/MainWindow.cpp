#include "MainWindow.h"

#include <QDockWidget>
#include <QMenuBar>
#include <QMessageBox>

#include "ControlPanel.h"
#include "SettingsDialog.h"
#include "TreeCanvas.h"
#include "core/version.h"

namespace D {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(QString("Meristem v") + MERISTEM_VERSION_STRING);
  resize(1280, 720);

  m_canvas = new TreeCanvas(this);
  m_canvas->setMinimumSize(640, 480);
  setCentralWidget(m_canvas);

  m_panel = new ControlPanel(m_canvas, this);

  auto* dock = new QDockWidget("Controls", this);
  dock->setWidget(m_panel);
  dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dock);

  createMenus();
}

void MainWindow::createMenus() {
  // ── Edit ──────────────────────────────────────────────────────────────────────

  auto* edit = menuBar()->addMenu("&Edit");

  auto* settingsAct = edit->addAction("Settings...");
  connect(settingsAct, &QAction::triggered, this, [this]() {
    SettingsDialog dlg(m_canvas, this);
    dlg.exec();
  });

  // ── Help ──────────────────────────────────────────────────────────────────────

  auto* help = menuBar()->addMenu("&Help");

  auto* aboutAct = help->addAction("About Meristem");
  connect(aboutAct, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, "About Meristem",
        QString("<b>Meristem</b> v%1<br><br>"
                "L-system plant modelling framework.")
            .arg(MERISTEM_VERSION_STRING));
  });
}

}  // namespace D
