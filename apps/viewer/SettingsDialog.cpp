#include "SettingsDialog.h"

#include <functional>

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "TreeCanvas.h"

namespace D {

namespace {

QPushButton* colorButton(QColor color, QWidget* parent,
                          std::function<void(QColor)> onChange) {
  auto* btn = new QPushButton(parent);
  btn->setMinimumWidth(60);
  btn->setFixedHeight(22);

  auto applyStyle = [btn](QColor c) {
    btn->setStyleSheet(
        QString("background-color: rgb(%1,%2,%3); border: 1px solid #555;")
            .arg(c.red()).arg(c.green()).arg(c.blue()));
  };
  applyStyle(color);

  QObject::connect(btn, &QPushButton::clicked, parent, [btn, applyStyle, onChange]() {
    QColor c = QColorDialog::getColor(
        btn->palette().button().color(), btn, "Pick color");
    if (c.isValid()) { applyStyle(c); onChange(c); }
  });

  return btn;
}

QLineEdit* symbolEdit(char ch, QWidget* parent) {
  auto* ed = new QLineEdit(QString(ch), parent);
  ed->setMaxLength(1);
  ed->setFixedWidth(36);
  ed->setAlignment(Qt::AlignCenter);
  return ed;
}

}  // namespace

SettingsDialog::SettingsDialog(TreeCanvas* canvas, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle("Preferences");
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  auto* lay  = new QVBoxLayout(this);
  auto* tabs = new QTabWidget;
  lay->addWidget(tabs);

  // ── Appearance tab ────────────────────────────────────────────────────────────

  auto* appearPage = new QWidget;
  auto* appearLay  = new QVBoxLayout(appearPage);

  auto* colorsBox  = new QGroupBox("Colors");
  auto* colorsForm = new QFormLayout(colorsBox);

  colorsForm->addRow("Line color:",
      colorButton(canvas->lineColor(), appearPage,
          [canvas](QColor c) { canvas->setLineColor(c); }));
  colorsForm->addRow("Background:",
      colorButton(canvas->bgColor(), appearPage,
          [canvas](QColor c) { canvas->setBgColor(c); }));
  colorsForm->addRow("Flower color:",
      colorButton(canvas->flowerColor(), appearPage,
          [canvas](QColor c) { canvas->setFlowerColor(c); }));

  appearLay->addWidget(colorsBox);

  auto* flowerBox  = new QGroupBox("Flower");
  auto* flowerForm = new QFormLayout(flowerBox);

  auto* radSpin = new QDoubleSpinBox;
  radSpin->setRange(0.05, 3.0);
  radSpin->setSingleStep(0.05);
  radSpin->setDecimals(2);
  radSpin->setValue(canvas->flowerRadius());
  connect(radSpin, &QDoubleSpinBox::valueChanged,
          canvas, &TreeCanvas::setFlowerRadius);
  flowerForm->addRow("Radius:", radSpin);

  appearLay->addWidget(flowerBox);
  appearLay->addStretch();

  tabs->addTab(appearPage, "Appearance");

  // ── Symbols tab ───────────────────────────────────────────────────────────────

  auto* symPage = new QWidget;
  auto* symForm = new QFormLayout(symPage);

  const TurtleSymbols initSym = canvas->symbols();
  auto* edForward    = symbolEdit(initSym.forward,       symPage);
  auto* edForwardND  = symbolEdit(initSym.forwardNoDraw, symPage);
  auto* edImmature   = symbolEdit(initSym.immature,      symPage);
  auto* edTurnLeft   = symbolEdit(initSym.turnLeft,      symPage);
  auto* edTurnRight  = symbolEdit(initSym.turnRight,     symPage);
  auto* edTurnAround = symbolEdit(initSym.turnAround,    symPage);
  auto* edPush       = symbolEdit(initSym.push,          symPage);
  auto* edPop        = symbolEdit(initSym.pop,           symPage);
  auto* edFlower     = symbolEdit(initSym.flower,        symPage);

  symForm->addRow("Forward (draw):",     edForward);
  symForm->addRow("Forward (no draw):",  edForwardND);
  symForm->addRow("Immature stem:",      edImmature);
  symForm->addRow("Turn left:",          edTurnLeft);
  symForm->addRow("Turn right:",         edTurnRight);
  symForm->addRow("Turn around (180°):", edTurnAround);
  symForm->addRow("Push state:",         edPush);
  symForm->addRow("Pop state:",          edPop);
  symForm->addRow("Flower:",             edFlower);

  // Collect all edits; rebuild TurtleSymbols and push to canvas on any change.
  auto applySymbols = [=]() {
    auto ch = [](QLineEdit* ed, char def) -> char {
      const QString t = ed->text();
      return t.isEmpty() ? def : t.at(0).toLatin1();
    };
    TurtleSymbols s;
    s.forward       = ch(edForward,    initSym.forward);
    s.forwardNoDraw = ch(edForwardND,  initSym.forwardNoDraw);
    s.immature      = ch(edImmature,   initSym.immature);
    s.turnLeft      = ch(edTurnLeft,   initSym.turnLeft);
    s.turnRight     = ch(edTurnRight,  initSym.turnRight);
    s.turnAround    = ch(edTurnAround, initSym.turnAround);
    s.push          = ch(edPush,       initSym.push);
    s.pop           = ch(edPop,        initSym.pop);
    s.flower        = ch(edFlower,     initSym.flower);
    canvas->setSymbols(s);
  };

  for (auto* ed : {edForward, edForwardND, edImmature, edTurnLeft,
                   edTurnRight, edTurnAround, edPush, edPop, edFlower}) {
    QObject::connect(ed, &QLineEdit::textChanged,
                     symPage, [applySymbols](const QString&) { applySymbols(); });
  }

  tabs->addTab(symPage, "Symbols");

  // ── Close ─────────────────────────────────────────────────────────────────────

  auto* closeBtn = new QPushButton("Close");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  lay->addWidget(closeBtn);
}

}  // namespace D
