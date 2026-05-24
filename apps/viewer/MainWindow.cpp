#include "MainWindow.h"

#include <QDockWidget>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "ControlPanel.h"
#include "LogWidget.h"
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
  m_controlsDock->setWidget(m_panel);
  m_controlsDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
                              QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::LeftDockWidgetArea, m_controlsDock);

  m_logWidget = new LogWidget(this);
  m_logDock   = new QDockWidget("Log", this);
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

}  // namespace D
