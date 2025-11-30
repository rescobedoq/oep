#include "ui/mainwindow.h"
#include <QApplication>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    #ifdef PROJECT_ROOT
        QDir::setCurrent(PROJECT_ROOT);
    #endif

    ui::MainWindow mainWindow;

    return app.exec();
}
