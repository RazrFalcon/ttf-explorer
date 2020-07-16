#include "app.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    MainWindow w;
    w.show();

    QObject::connect(&app, &Application::openFile, &w, &MainWindow::loadFile);

    return app.exec();
}
