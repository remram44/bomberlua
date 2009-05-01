#include <QApplication>
#include <QLocale>

#include "CLauncher.h"

#include <iostream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    CLauncher launcher;

    launcher.show();

    return app.exec();
}
