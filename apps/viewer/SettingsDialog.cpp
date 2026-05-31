#include "SettingsDialog.h"

#include <functional>
#include <memory>
#include <vector>

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "TreeCanvas.h"

namespace D {

namespace {

// A color swatch button. Reads its initial color from get(), writes new picks
// via set(), and registers a refresher so it can be re-synced from the canvas
// (e.g. after "Restore Defaults").
QPushButton* colorButton(std::function<QColor()> get, std::function<void(QColor)> set,
                         std::vector<std::function<void()>>& refreshers, QWidget* parent) {
  auto* btn = new QPushButton(parent);
  btn->setMinimumWidth(60);
  btn->setFixedHeight(22);

  auto applyStyle = [btn](QColor c) {
    btn->setStyleSheet(
        QString("background-color: rgb(%1,%2,%3); border: 1px solid #555;")
            .arg(c.red()).arg(c.green()).arg(c.blue()));
  };
  applyStyle(get());

  // Pass btn->window() (the Preferences dialog) as parent, not btn itself.
  // If btn were passed, Qt would cascade btn's background-color stylesheet
  // onto QColorDialog, tinting its entire background.
  QObject::connect(btn, &QPushButton::clicked, parent, [btn, get, set, applyStyle]() {
    QColor c = QColorDialog::getColor(get(), btn->window(), "Pick color");
    if (c.isValid()) { applyStyle(c); set(c); }
  });

  refreshers.push_back([get, applyStyle]() { applyStyle(get()); });
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

  // Refreshers re-read every widget's value from the canvas; "Restore Defaults"
  // resets the canvas then runs them all.
  auto refreshers = std::make_shared<std::vector<std::function<void()>>>();

  auto* lay  = new QVBoxLayout(this);
  auto* tabs = new QTabWidget;
  lay->addWidget(tabs);

  // ── Appearance tab ────────────────────────────────────────────────────────────

  auto* appearPage = new QWidget;
  auto* appearLay  = new QVBoxLayout(appearPage);

  auto* colorsBox  = new QGroupBox("Colors");
  auto* colorsForm = new QFormLayout(colorsBox);

  colorsForm->addRow("Line color:",
      colorButton([canvas] { return canvas->lineColor(); },
                  [canvas](QColor c) { canvas->setLineColor(c); }, *refreshers, appearPage));
  colorsForm->addRow("Background:",
      colorButton([canvas] { return canvas->bgColor(); },
                  [canvas](QColor c) { canvas->setBgColor(c); }, *refreshers, appearPage));
  colorsForm->addRow("Flower color:",
      colorButton([canvas] { return canvas->flowerColor(); },
                  [canvas](QColor c) { canvas->setFlowerColor(c); }, *refreshers, appearPage));

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
  refreshers->push_back([radSpin, canvas]() {
    QSignalBlocker b(radSpin);
    radSpin->setValue(canvas->flowerRadius());
  });

  appearLay->addWidget(flowerBox);
  appearLay->addStretch();

  tabs->addTab(appearPage, "Appearance");

  // ── Symbols tab ───────────────────────────────────────────────────────────────

  auto* symPage = new QWidget;
  auto* symForm = new QFormLayout(symPage);

  const TurtleSymbols initSym = canvas->symbols();
  auto* edForward    = symbolEdit(initSym.forward,       symPage);
  auto* edForwardND  = symbolEdit(initSym.forwardNoDraw, symPage);
  auto* edTurnLeft   = symbolEdit(initSym.turnLeft,      symPage);
  auto* edTurnRight  = symbolEdit(initSym.turnRight,     symPage);
  auto* edTurnAround = symbolEdit(initSym.turnAround,    symPage);
  auto* edPush       = symbolEdit(initSym.push,          symPage);
  auto* edPop        = symbolEdit(initSym.pop,           symPage);
  auto* edFlower     = symbolEdit(initSym.flower,        symPage);

  symForm->addRow("Forward (draw):",     edForward);
  symForm->addRow("Forward (no draw):",  edForwardND);
  symForm->addRow("Turn left:",          edTurnLeft);
  symForm->addRow("Turn right:",         edTurnRight);
  symForm->addRow("Turn around (180°):", edTurnAround);
  symForm->addRow("Push state:",         edPush);
  symForm->addRow("Pop state:",          edPop);
  symForm->addRow("Flower:",             edFlower);

  const std::vector<QLineEdit*> symEdits = {edForward, edForwardND, edTurnLeft, edTurnRight,
                                            edTurnAround, edPush, edPop, edFlower};

  // Collect all edits; rebuild TurtleSymbols and push to canvas on any change.
  auto applySymbols = [=]() {
    auto ch = [](QLineEdit* ed, char def) -> char {
      const QString t = ed->text();
      return t.isEmpty() ? def : t.at(0).toLatin1();
    };
    TurtleSymbols s;
    s.forward       = ch(edForward,    initSym.forward);
    s.forwardNoDraw = ch(edForwardND,  initSym.forwardNoDraw);
    s.turnLeft      = ch(edTurnLeft,   initSym.turnLeft);
    s.turnRight     = ch(edTurnRight,  initSym.turnRight);
    s.turnAround    = ch(edTurnAround, initSym.turnAround);
    s.push          = ch(edPush,       initSym.push);
    s.pop           = ch(edPop,        initSym.pop);
    s.flower        = ch(edFlower,     initSym.flower);
    canvas->setSymbols(s);
  };

  for (auto* ed : symEdits) {
    QObject::connect(ed, &QLineEdit::textChanged,
                     symPage, [applySymbols](const QString&) { applySymbols(); });
  }

  refreshers->push_back([symEdits, canvas]() {
    const TurtleSymbols s = canvas->symbols();
    const char chars[] = {s.forward, s.forwardNoDraw, s.turnLeft, s.turnRight,
                          s.turnAround, s.push, s.pop, s.flower};
    for (std::size_t i = 0; i < symEdits.size(); ++i) {
      QSignalBlocker b(symEdits[i]);
      symEdits[i]->setText(QString(QChar(chars[i])));
    }
  });

  tabs->addTab(symPage, "Symbols");

  // ── Buttons ─────────────────────────────────────────────────────────────────────

  auto* btnRow = new QHBoxLayout;
  auto* resetBtn = new QPushButton("Restore Defaults");
  connect(resetBtn, &QPushButton::clicked, this, [canvas, refreshers]() {
    canvas->restoreDefaultAppearance();
    for (auto& r : *refreshers) r();
  });
  btnRow->addWidget(resetBtn);
  btnRow->addStretch();

  auto* closeBtn = new QPushButton("Close");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  btnRow->addWidget(closeBtn);

  lay->addLayout(btnRow);
}

}  // namespace D
