#include "SettingsDialog.h"

#include <functional>

#include <QCloseEvent>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

#include "Preferences.h"
#include "TreeCanvas.h"

namespace D {

namespace {

// A color swatch button. Reads its color from get(), writes picks via set(),
// flags the dialog dirty, and registers a refresher so it can be re-synced from
// the canvas (e.g. after "Restore Defaults" or a revert).
QPushButton* colorButton(std::function<QColor()> get, std::function<void(QColor)> set,
                         std::function<void()> onEdit,
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

  // Pass btn->window() (the dialog) as parent, not btn itself, or Qt cascades
  // btn's background-color stylesheet onto QColorDialog.
  QObject::connect(btn, &QPushButton::clicked, parent, [btn, get, set, applyStyle, onEdit]() {
    QColor c = QColorDialog::getColor(get(), btn->window(), "Pick color");
    if (c.isValid()) { applyStyle(c); set(c); onEdit(); }
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
    : QDialog(parent), m_canvas(canvas) {
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setDirty(false);  // sets the initial "Preferences" title

  captureSnapshot();

  auto onEdit = [this]() { markDirty(); };

  auto* lay  = new QVBoxLayout(this);
  auto* tabs = new QTabWidget;
  lay->addWidget(tabs);

  // ── Appearance tab ────────────────────────────────────────────────────────────

  auto* appearPage = new QWidget;
  auto* appearLay  = new QVBoxLayout(appearPage);

  auto* colorsBox  = new QGroupBox("Colors");
  auto* colorsForm = new QFormLayout(colorsBox);

  colorsForm->addRow("Line color:",
      colorButton([this] { return m_canvas->lineColor(); },
                  [this](QColor c) { m_canvas->setLineColor(c); }, onEdit, m_refreshers, appearPage));
  colorsForm->addRow("Background:",
      colorButton([this] { return m_canvas->bgColor(); },
                  [this](QColor c) { m_canvas->setBgColor(c); }, onEdit, m_refreshers, appearPage));
  colorsForm->addRow("Flower color:",
      colorButton([this] { return m_canvas->flowerColor(); },
                  [this](QColor c) { m_canvas->setFlowerColor(c); }, onEdit, m_refreshers, appearPage));

  appearLay->addWidget(colorsBox);

  auto* flowerBox  = new QGroupBox("Flower");
  auto* flowerForm = new QFormLayout(flowerBox);

  auto* radSpin = new QDoubleSpinBox;
  radSpin->setRange(0.05, 3.0);
  radSpin->setSingleStep(0.05);
  radSpin->setDecimals(2);
  radSpin->setValue(m_canvas->flowerRadius());
  connect(radSpin, &QDoubleSpinBox::valueChanged, this, [this](double v) {
    m_canvas->setFlowerRadius(v);
    markDirty();
  });
  flowerForm->addRow("Radius:", radSpin);
  m_refreshers.push_back([radSpin, this]() {
    QSignalBlocker b(radSpin);
    radSpin->setValue(m_canvas->flowerRadius());
  });

  appearLay->addWidget(flowerBox);
  appearLay->addStretch();

  tabs->addTab(appearPage, "Appearance");

  // ── Symbols tab ───────────────────────────────────────────────────────────────

  auto* symPage = new QWidget;
  auto* symForm = new QFormLayout(symPage);

  const TurtleSymbols initSym = m_canvas->symbols();
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
    m_canvas->setSymbols(s);
  };

  for (auto* ed : symEdits) {
    connect(ed, &QLineEdit::textChanged, this, [this, applySymbols](const QString&) {
      applySymbols();
      markDirty();
    });
  }

  m_refreshers.push_back([symEdits, this]() {
    const TurtleSymbols s = m_canvas->symbols();
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
  connect(resetBtn, &QPushButton::clicked, this, [this]() {
    m_canvas->restoreDefaultAppearance();
    for (auto& r : m_refreshers) r();
    markDirty();
  });
  btnRow->addWidget(resetBtn);
  btnRow->addStretch();

  auto* saveBtn = new QPushButton("Save");
  saveBtn->setDefault(true);
  connect(saveBtn, &QPushButton::clicked, this, [this]() {
    commit();
    accept();
  });
  btnRow->addWidget(saveBtn);

  auto* closeBtn = new QPushButton("Close");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
  btnRow->addWidget(closeBtn);

  lay->addLayout(btnRow);
}

void SettingsDialog::setDirty(bool dirty) {
  m_dirty = dirty;
  setWindowTitle(dirty ? "Preferences*" : "Preferences");
}

void SettingsDialog::captureSnapshot() {
  m_snapshot.line = m_canvas->lineColor();
  m_snapshot.flower = m_canvas->flowerColor();
  m_snapshot.bg = m_canvas->bgColor();
  m_snapshot.flowerRadius = m_canvas->flowerRadius();
  m_snapshot.symbols = m_canvas->symbols();
}

void SettingsDialog::revertToSnapshot() {
  m_canvas->setLineColor(m_snapshot.line);
  m_canvas->setFlowerColor(m_snapshot.flower);
  m_canvas->setBgColor(m_snapshot.bg);
  m_canvas->setFlowerRadius(m_snapshot.flowerRadius);
  m_canvas->setSymbols(m_snapshot.symbols);
  for (auto& r : m_refreshers) r();
}

void SettingsDialog::commit() {
  // Live edits already updated the canvas; persist the current appearance.
  savePreferences(*m_canvas);
  setDirty(false);
}

bool SettingsDialog::confirmClose() {
  if (!m_dirty) return true;
  const auto choice = QMessageBox::warning(
      this, "Preferences", "Apply changes to preferences?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  if (choice == QMessageBox::Save) { commit(); return true; }
  if (choice == QMessageBox::Discard) { revertToSnapshot(); setDirty(false); return true; }
  return false;  // Cancel
}

void SettingsDialog::closeEvent(QCloseEvent* event) {
  if (confirmClose()) event->accept();
  else event->ignore();
}

void SettingsDialog::reject() {
  // Esc and the Close button funnel through the same save/discard prompt.
  if (confirmClose()) QDialog::reject();
}

}  // namespace D
