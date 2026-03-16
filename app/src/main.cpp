#include "MainWindow.h"

#include <QApplication>
#include <QIcon>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // Disable multisampling in Qt BEFORE QApplication
    QSurfaceFormat fmt;
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
