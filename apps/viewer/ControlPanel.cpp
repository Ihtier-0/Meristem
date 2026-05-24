#include "ControlPanel.h"

#include <QVBoxLayout>

#include "GrammarWidget.h"
#include "LSystemWidget.h"
#include "TreeCanvas.h"

namespace D {

ControlPanel::ControlPanel(TreeCanvas* canvas, QWidget* parent) : QScrollArea(parent) {
  setWidgetResizable(true);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setMinimumWidth(360);
  setMaximumWidth(380);

  auto* root = new QWidget;
  auto* lay = new QVBoxLayout(root);
  lay->setSpacing(6);
  lay->setContentsMargins(6, 6, 6, 6);

  m_lsystem = new LSystemWidget(canvas, this);
  m_grammar = new GrammarWidget(canvas, this);
  lay->addWidget(m_lsystem);
  lay->addWidget(m_grammar);
  lay->addStretch();

  setWidget(root);

  connect(canvas, &TreeCanvas::algoSwitched, m_lsystem, &LSystemWidget::onAlgoSwitched);
  connect(canvas, &TreeCanvas::algoSwitched, m_grammar, &GrammarWidget::onAlgoSwitched);
  connect(canvas, &TreeCanvas::viewChanged, m_lsystem, &LSystemWidget::onViewChanged);
}

}  // namespace D
