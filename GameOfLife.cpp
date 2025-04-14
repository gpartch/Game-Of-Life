#include "Viewer.hpp"

#include <QApplication>

int qMain(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Viewer v;
    //v.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    v.show();
    return app.exec();
}
