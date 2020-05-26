#include <QApplication>

#include "mainwindow.h"
#include "ui.h"

struct ttfe_ui_app
{
    MainWindow *mainwindow;
};

ttfe_ui_app* ttfe_ui_init()
{
    if (!QApplication::instance()) {
        static int argc = 1;
        static char *argv[] = { const_cast<char*>("ttf-explorer") };
        new QApplication(argc, argv);
    }

    // Those object will be deallocated on app close,
    // so there is no point in deallocating them manually.
    auto app = new ttfe_ui_app();
    app->mainwindow = new MainWindow();
    return app;
}

void ttfe_ui_exec(ttfe_ui_app *app, const char *path, uint len)
{
    app->mainwindow->show();
    if (path != nullptr) {
        app->mainwindow->loadFile(QString::fromUtf8(path, len));
    }

    qApp->exec();
}
