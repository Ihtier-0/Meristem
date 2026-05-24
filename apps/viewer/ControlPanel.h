#pragma once

#include <QScrollArea>

namespace D {

class GrammarWidget;
class LSystemWidget;
class TreeCanvas;

class ControlPanel final : public QScrollArea {
  Q_OBJECT

 public:
  explicit ControlPanel(TreeCanvas* canvas, QWidget* parent = nullptr);

 private:
  LSystemWidget* m_lsystem = nullptr;
  GrammarWidget* m_grammar = nullptr;
};

}  // namespace D
