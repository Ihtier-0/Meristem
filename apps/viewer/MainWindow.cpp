#include "MainWindow.h"

#include <QColorDialog>
#include <QDockWidget>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>

#include "ControlPanel.h"
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
  // ── Settings ──────────────────────────────────────────────────────────────────

  auto* settings = menuBar()->addMenu("&Settings");

  auto* lineColorAct = settings->addAction("Line Color...");
  connect(lineColorAct, &QAction::triggered, this, [this]() {
    QColor c = QColorDialog::getColor(m_canvas->lineColor(), this, "Line Color");
    if (c.isValid()) m_canvas->setLineColor(c);
  });

  auto* bgColorAct = settings->addAction("Background Color...");
  connect(bgColorAct, &QAction::triggered, this, [this]() {
    QColor c = QColorDialog::getColor(m_canvas->bgColor(), this, "Background Color");
    if (c.isValid()) m_canvas->setBgColor(c);
  });

  auto* flowerColorAct = settings->addAction("Flower Color...");
  connect(flowerColorAct, &QAction::triggered, this, [this]() {
    QColor c = QColorDialog::getColor(m_canvas->flowerColor(), this, "Flower Color");
    if (c.isValid()) m_canvas->setFlowerColor(c);
  });

  settings->addSeparator();

  auto* radAct = settings->addAction("Flower Radius...");
  connect(radAct, &QAction::triggered, this, [this]() {
    bool ok = false;
    double r = QInputDialog::getDouble(
        this, "Flower Radius", "Radius:",
        m_canvas->flowerRadius(), 0.05, 3.0, 2, &ok);
    if (ok) m_canvas->setFlowerRadius(r);
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
