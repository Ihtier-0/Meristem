#include "Preferences.h"

#include <QColor>
#include <QSettings>
#include <QString>

#include "TreeCanvas.h"

namespace D {

void loadPreferences(TreeCanvas& c) {
  QSettings s;

  s.beginGroup("appearance");
  if (s.contains("lineColor")) c.setLineColor(s.value("lineColor").value<QColor>());
  if (s.contains("flowerColor")) c.setFlowerColor(s.value("flowerColor").value<QColor>());
  if (s.contains("bgColor")) c.setBgColor(s.value("bgColor").value<QColor>());
  if (s.contains("flowerRadius")) c.setFlowerRadius(s.value("flowerRadius").toDouble());
  s.endGroup();

  s.beginGroup("symbols");
  TurtleSymbols sym = c.symbols();
  auto rd = [&](const char* key, char& dst) {
    if (!s.contains(key)) return;
    const QString v = s.value(key).toString();
    if (!v.isEmpty()) dst = v[0].toLatin1();
  };
  rd("forward", sym.forward);
  rd("forwardNoDraw", sym.forwardNoDraw);
  rd("turnLeft", sym.turnLeft);
  rd("turnRight", sym.turnRight);
  rd("turnAround", sym.turnAround);
  rd("push", sym.push);
  rd("pop", sym.pop);
  rd("flower", sym.flower);
  s.endGroup();
  c.setSymbols(sym);
}

void savePreferences(const TreeCanvas& c) {
  QSettings s;

  s.beginGroup("appearance");
  s.setValue("lineColor", c.lineColor());
  s.setValue("flowerColor", c.flowerColor());
  s.setValue("bgColor", c.bgColor());
  s.setValue("flowerRadius", c.flowerRadius());
  s.endGroup();

  const TurtleSymbols sym = c.symbols();
  s.beginGroup("symbols");
  auto wr = [&](const char* key, char ch) { s.setValue(key, QString(QChar(ch))); };
  wr("forward", sym.forward);
  wr("forwardNoDraw", sym.forwardNoDraw);
  wr("turnLeft", sym.turnLeft);
  wr("turnRight", sym.turnRight);
  wr("turnAround", sym.turnAround);
  wr("push", sym.push);
  wr("pop", sym.pop);
  wr("flower", sym.flower);
  s.endGroup();
}

}  // namespace D
