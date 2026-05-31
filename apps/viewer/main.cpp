#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

#include "MainWindow.h"
#include "core/meristem.h"

// Pixel-tree icon drawn with QPainter — no image plugin required.
// Design on a 16x16 grid, scaled to each requested size:
//
//   row 0:  ...XX......XX...   branch tips
//   row 1:  ....XX....XX....
//   row 2:  .....XX..XX.....
//   row 3:  ......XXXX......   junction
//   row 4-15: .......XX.......   trunk
//
static QIcon createAppIcon() {
  const int sizes[] = {16, 32, 48, 64};
  QIcon icon;
  for (int sz : sizes) {
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0x4a, 0xde, 0x80));  // #4ade80

    const float s = static_cast<float>(sz) / 16.f;
    auto block = [&](float x, float y, float w, float h) {
      p.drawRect(QRectF(x * s, y * s, w * s, h * s).toAlignedRect());
    };

    block(3,  0, 2, 1);  // left branch tip
    block(4,  1, 2, 1);
    block(5,  2, 2, 1);
    block(11, 0, 2, 1);  // right branch tip
    block(10, 1, 2, 1);
    block(9,  2, 2, 1);
    block(6,  3, 4, 1);  // junction
    block(7,  4, 2, 12); // trunk

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
