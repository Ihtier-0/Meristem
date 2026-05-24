#include "SettingsDialog.h"

#include <functional>

#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
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

}  // namespace

SettingsDialog::SettingsDialog(TreeCanvas* canvas, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle("Settings");
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  auto* lay = new QVBoxLayout(this);

  // ── Colors ───────────────────────────────────────────────────────────────────

  auto* colorsBox  = new QGroupBox("Colors");
  auto* colorsForm = new QFormLayout(colorsBox);

  colorsForm->addRow("Line color:",
      colorButton(canvas->lineColor(), this,
          [canvas](QColor c) { canvas->setLineColor(c); }));

  colorsForm->addRow("Background:",
      colorButton(canvas->bgColor(), this,
          [canvas](QColor c) { canvas->setBgColor(c); }));

  colorsForm->addRow("Flower color:",
      colorButton(canvas->flowerColor(), this,
          [canvas](QColor c) { canvas->setFlowerColor(c); }));

  lay->addWidget(colorsBox);

  // ── Flower ───────────────────────────────────────────────────────────────────

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

  lay->addWidget(flowerBox);

  // ── Close ────────────────────────────────────────────────────────────────────

  auto* closeBtn = new QPushButton("Close");
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
  lay->addWidget(closeBtn);
}

}  // namespace D
