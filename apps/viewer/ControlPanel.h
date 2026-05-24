#pragma once

#include <QScrollArea>

namespace D {

class TreeCanvas;
class LSystemWidget;
class GrammarWidget;

class ControlPanel : public QScrollArea {
  Q_OBJECT

 public:
  explicit ControlPanel(TreeCanvas* canvas, QWidget* parent = nullptr);

 private:
  LSystemWidget* m_lsystem = nullptr;
  GrammarWidget* m_grammar = nullptr;
};

}  // namespace D
