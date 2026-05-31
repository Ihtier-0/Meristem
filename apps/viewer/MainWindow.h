#pragma once

#include <QMainWindow>

class QCloseEvent;
class QDockWidget;
class QLabel;

namespace D {

class ControlPanel;
class LogWidget;
class TreeCanvas;

class MainWindow final : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

 protected:
  void closeEvent(QCloseEvent* event) override;

 private:
  void createMenus();
  void refreshStatus();
  void setupLogging();
  void updateTitle();
  void openPlant();
  bool savePlant(bool saveAs);     // returns true on success; false if cancelled/failed
  bool maybeSaveDocument();        // prompt to save a modified document before discarding

  TreeCanvas*   m_canvas       = nullptr;
  ControlPanel* m_panel        = nullptr;
  QLabel*       m_statusLabel  = nullptr;
  QDockWidget*  m_controlsDock = nullptr;
  QDockWidget*  m_logDock      = nullptr;
  LogWidget*    m_logWidget    = nullptr;
};

}  // namespace D
