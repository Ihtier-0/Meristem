#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include "ControlPanel.h"
#include "SettingsDialog.h"
#include "TreeCanvas.h"
#include "core/meristem.h"

namespace D {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
  setWindowTitle(QString(MERISTEM_PROJECT_NAME) + " v" + MERISTEM_VERSION_STRING);
  resize(1280, 720);

  m_canvas = new TreeCanvas(this);
  m_canvas->setMinimumSize(640, 480);
  setCentralWidget(m_canvas);

  m_panel = new ControlPanel(m_canvas, this);

  auto* dock = new QDockWidget("Controls", this);
  dock->setWidget(m_panel);
  dock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, dock);

  // ── Status bar ────────────────────────────────────────────────────────────────

  m_statusLabel = new QLabel;
  statusBar()->addWidget(m_statusLabel);
  connect(m_canvas, &TreeCanvas::stateChanged,
          this, [this](int, int) { refreshStatus(); });
  connect(m_canvas, &TreeCanvas::viewChanged,
          this, [this](double, double, double) { refreshStatus(); });
  refreshStatus();

  createMenus();
}

void MainWindow::createMenus() {
  // ── Edit ──────────────────────────────────────────────────────────────────────

  auto* edit = menuBar()->addMenu("&Edit");

  auto* prefsAct = edit->addAction("Preferences...");
  connect(prefsAct, &QAction::triggered, this, [this]() {
    SettingsDialog dlg(m_canvas, this);
    dlg.exec();
  });

  // ── Help ──────────────────────────────────────────────────────────────────────

  auto* help = menuBar()->addMenu("&Help");

  auto* aboutAct = help->addAction("About Meristem");
  connect(aboutAct, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, QString("About ") + MERISTEM_PROJECT_NAME,
        QString("<b>%1</b> v%2<br><br>"
                "L-system plant modelling framework.")
            .arg(MERISTEM_PROJECT_NAME).arg(MERISTEM_VERSION_STRING));
  });
}

void MainWindow::refreshStatus() {
  m_statusLabel->setText(
      QString("Gen: %1   |   Symbols: %2   |   Zoom: %3×")
          .arg(m_canvas->generation())
          .arg(m_canvas->symbolCount())
          .arg(m_canvas->zoom(), 0, 'f', 2));
}

}  // namespace D
