#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

#include "MainWindow.h"
#include "core/meristem.h"

// Pixel-tree icon — matches the SVG in apps/viewer/icons/icon.svg.
// The SVG uses a 64x64 viewBox with 8x8 blocks; we scale them to each size.
//
//   col:  1    3.5  6        (outer / center / outer)
//   row 1: X    X    X       level 4 (top)
//   row 2:  X   X   X        level 3
//   row 3: X    X    X       level 2
//   row 4:  X   X   X        level 1
//   row 5:      X            trunk
//   row 6:      X
//   row 7:      X
//
static QIcon createAppIcon() {
  // Block positions in the original 64x64 coordinate space (x, y), all 8x8.
  struct Block { float x, y; };
  constexpr Block blocks[] = {
    // level 4 — top
    {8,  8}, {28, 8}, {48, 8},
    // level 3
    {16, 16}, {28, 16}, {40, 16},
    // level 2
    {8,  24}, {28, 24}, {48, 24},
    // level 1
    {16, 32}, {28, 32}, {40, 32},
    // trunk
    {28, 40}, {28, 48}, {28, 56},
  };

  const int sizes[] = {16, 32, 48, 64};
  QIcon icon;
  for (int sz : sizes) {
    QPixmap pm(sz, sz);
    pm.fill(QColor(0x1a, 0x1a, 0x1a));  // dark background like the SVG
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x4a, 0xde, 0x80));  // #4ade80

    const float s = static_cast<float>(sz) / 64.f;
    for (const auto& b : blocks)
      p.drawRect(QRectF(b.x * s, b.y * s, 8 * s, 8 * s).toAlignedRect());

    icon.addPixmap(pm);
  }
  return icon;
}

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(MERISTEM_PROJECT_NAME);
  app.setApplicationVersion(MERISTEM_VERSION_STRING);
  app.setWindowIcon(createAppIcon());

  D::MainWindow window;
  window.show();

  return app.exec();
}
