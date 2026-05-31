#pragma once

#include <functional>
#include <vector>

#include <QColor>
#include <QDialog>

#include "geometry/TurtleBuilder2D.h"

namespace D {

class TreeCanvas;

// Preferences dialog with proper Save / Close semantics:
//   - edits preview live on the canvas (so the viewport updates immediately);
//   - Save commits them and persists to QSettings;
//   - closing with unsaved edits prompts, and Discard reverts the canvas to the
//     snapshot taken when the dialog opened.
class SettingsDialog final : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(TreeCanvas* canvas, QWidget* parent = nullptr);

 protected:
  void closeEvent(QCloseEvent* event) override;
  void reject() override;  // Esc / Close button route through the same prompt

 private:
  struct Snapshot {
    QColor line, flower, bg;
    double flowerRadius = 0.3;
    TurtleSymbols symbols;
  };

  void captureSnapshot();
  void revertToSnapshot();
  void commit();             // persist to QSettings, clear dirty
  bool confirmClose();       // true if the dialog may close
  void setDirty(bool dirty); // update flag and reflect "*" in the title
  void markDirty() { setDirty(true); }

  TreeCanvas* m_canvas = nullptr;
  Snapshot m_snapshot;
  bool m_dirty = false;
  std::vector<std::function<void()>> m_refreshers;
};

}  // namespace D
