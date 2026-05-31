#include <QApplication>
#include <QIcon>

#include "MainWindow.h"
#include "core/meristem.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(MERISTEM_PROJECT_NAME);
  app.setApplicationVersion(MERISTEM_VERSION_STRING);
  app.setWindowIcon(QIcon(":/icons/icon.svg"));

  D::MainWindow window;
  window.show();

  return app.exec();
}
