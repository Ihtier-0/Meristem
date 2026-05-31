#include <QApplication>
#include <QIcon>
#include <QPainter>
#include <QPixmap>

#include "MainWindow.h"
#include "core/meristem.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setOrganizationName(MERISTEM_PROJECT_NAME);  // QSettings store key
  app.setApplicationName(MERISTEM_PROJECT_NAME);
  app.setApplicationVersion(MERISTEM_VERSION_STRING);
  app.setWindowIcon(QIcon(":/icons/icon.svg"));

  D::MainWindow window;
  window.show();

  return app.exec();
}
