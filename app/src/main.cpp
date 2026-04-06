#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // Request OpenGL 3.3 Core Profile BEFORE QApplication
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(0);
    QSurfaceFormat::setDefaultFormat(fmt);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/spacecraft-telemetry-icon.svg"));
    app.setApplicationName("Spacecraft Telemetry Visualizer");
    app.setDesktopFileName("spacecraft-telemetry-viz");

    MainWindow w;
    w.show();
    return app.exec();
}
