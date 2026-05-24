#include <QApplication>

#include "MainWindow.h"
#include "core/meristem.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(MERISTEM_PROJECT_NAME);

  D::MainWindow window;
  window.show();

  return app.exec();
}
