#include "MainWindow.h"

#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "ControlPanel.h"
#include "LogWidget.h"
#include "PlantIO.h"
#include "Preferences.h"
#include "QtLogSink.h"
#include "SettingsDialog.h"
#include "TreeCanvas.h"

#include "core/meristem.h"

namespace D {

MainWindow::MainWindow(QWidget* parent /* = nullptr */,
                       Qt::WindowFlags flags /* = Qt::WindowFlags() */)
    : QMainWindow(parent, flags) {
  setWindowTitle(QString(MERISTEM_PROJECT_NAME) + " v" + MERISTEM_VERSION_STRING);
  resize(1280, 720);

  m_canvas = new TreeCanvas(this);
  m_canvas->setMinimumSize(640, 480);
  setCentralWidget(m_canvas);

  m_panel        = new ControlPanel(m_canvas, this);
  m_controlsDock = new QDockWidget("Properties", this);
  m_controlsDock->setObjectName("PropertiesDock");  // required for saveState/restoreState
  m_controlsDock->setWidget(m_panel);
  m_controlsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, m_controlsDock);

  m_logWidget = new LogWidget(this);
  m_logDock   = new QDockWidget("Log", this);
  m_logDock->setObjectName("LogDock");  // required for saveState/restoreState
  m_logDock->setWidget(m_logWidget);
  m_logDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
                         QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
  m_logDock->hide();

  setupLogging();

  m_statusLabel = new QLabel;
  statusBar()->addWidget(m_statusLabel);
  connect(m_canvas, &TreeCanvas::stateChanged, this, [this](int, int) { refreshStatus(); });
  connect(m_canvas, &TreeCanvas::viewChanged, this,
          [this](double, double, double) { refreshStatus(); });
  refreshStatus();

  createMenus();

  // ── Restore session ───────────────────────────────────────────────────────────
  QSettings settings;
  const QByteArray geom = settings.value("window/geometry").toByteArray();
  if (!geom.isEmpty()) {
    restoreGeometry(geom);
    restoreState(settings.value("window/state").toByteArray());
  }
  loadPreferences(*m_canvas);
}

void MainWindow::setupLogging() {
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("[%H:%M:%S.%e] %^[%l]%$ %v");

  auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("meristem.log", true);
  file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

  auto qt_sink = std::make_shared<QtLogSink>(m_logWidget);
  qt_sink->set_pattern("[%H:%M:%S] [%l] %v");

  auto logger = std::make_shared<spdlog::logger>(
      MERISTEM_PROJECT_NAME,
      spdlog::sinks_init_list{console_sink, file_sink, qt_sink});
  spdlog::set_default_logger(logger);
  spdlog::set_level(spdlog::level::info);

  spdlog::info("{} v{} started", MERISTEM_PROJECT_NAME, MERISTEM_VERSION_STRING);
}

void MainWindow::createMenus() {
  // ── File ──────────────────────────────────────────────────────────────────────

  auto* file     = menuBar()->addMenu("&File");
  auto* openAct  = file->addAction("&Open...");
  openAct->setShortcut(QKeySequence::Open);
  connect(openAct, &QAction::triggered, this, &MainWindow::openPlant);
  auto* saveAct  = file->addAction("&Save As...");
  saveAct->setShortcut(QKeySequence::Save);
  connect(saveAct, &QAction::triggered, this, &MainWindow::savePlant);

  // ── Edit ──────────────────────────────────────────────────────────────────────

  auto* edit      = menuBar()->addMenu("&Edit");
  auto* prefsAct  = edit->addAction("Preferences...");
  connect(prefsAct, &QAction::triggered, this, [this]() {
    SettingsDialog dlg(m_canvas, this);
    dlg.exec();
  });

  // ── Panels ────────────────────────────────────────────────────────────────────

  auto* panels = menuBar()->addMenu("&Panels");
  panels->addAction(m_controlsDock->toggleViewAction());
  panels->addAction(m_logDock->toggleViewAction());

  // ── Help ──────────────────────────────────────────────────────────────────────

  auto* help     = menuBar()->addMenu("&Help");
  auto* aboutAct = help->addAction("About " MERISTEM_PROJECT_NAME);
  connect(aboutAct, &QAction::triggered, this, [this]() {
    QMessageBox::about(this, QString("About ") + MERISTEM_PROJECT_NAME,
                       QString("<b>%1</b> v%2<br><br>"
                               "L-system plant modelling framework.")
                           .arg(MERISTEM_PROJECT_NAME)
                           .arg(MERISTEM_VERSION_STRING));
  });
}

void MainWindow::refreshStatus() {
  m_statusLabel->setText(QString("Gen: %1   |   Symbols: %2   |   Zoom: %3x")
                             .arg(m_canvas->generation())
                             .arg(m_canvas->symbolCount())
                             .arg(m_canvas->zoom(), 0, 'f', 3));
}

void MainWindow::openPlant() {
  const QString path =
      QFileDialog::getOpenFileName(this, "Open plant", {}, "Meristem plant (*.dt)");
  if (path.isEmpty()) return;
  QString err;
  if (loadPlantFile(*m_canvas, path, &err))
    spdlog::info("[Plant] Opened {}", path.toStdString());
  else
    QMessageBox::warning(this, "Open failed", err);
}

void MainWindow::savePlant() {
  QString path = QFileDialog::getSaveFileName(this, "Save plant", {}, "Meristem plant (*.dt)");
  if (path.isEmpty()) return;
  if (!path.endsWith(".dt", Qt::CaseInsensitive)) path += ".dt";
  QString err;
  if (savePlantFile(*m_canvas, path, &err))
    spdlog::info("[Plant] Saved {}", path.toStdString());
  else
    QMessageBox::warning(this, "Save failed", err);
}

void MainWindow::closeEvent(QCloseEvent* event) {
  QSettings settings;
  settings.setValue("window/geometry", saveGeometry());
  settings.setValue("window/state", saveState());
  savePreferences(*m_canvas);
  QMainWindow::closeEvent(event);
}

}  // namespace D
