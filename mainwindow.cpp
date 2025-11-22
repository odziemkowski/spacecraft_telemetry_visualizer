#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <QSurfaceFormat>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Prefer embedded Qt resource, fall back to filesystem locations
    const QString resourcePath = ":/icons/spacecraft-telemetry-icon.svg";
    if (QFile::exists(resourcePath) || QIcon::hasThemeIcon(resourcePath)) {
        setWindowIcon(QIcon(resourcePath));
    } else {
        QStringList iconCandidates = {
            QCoreApplication::applicationDirPath() + "/icons/spacecraft-telemetry-icon.svg",
            QCoreApplication::applicationDirPath() + "/../icons/spacecraft-telemetry-icon.svg",
            QCoreApplication::applicationDirPath() + "/../../icons/spacecraft-telemetry-icon.svg",
            "icons/spacecraft-telemetry-icon.svg"
        };
        for (const QString &p : iconCandidates) {
            if (QFile::exists(p)) {
                setWindowIcon(QIcon(p));
                break;
            }
        }
    }

    setupVTK();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupVTK()
{
    // Ensure the Qt surface format is compatible with VTK's OpenGL requirements
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    // Create the QVTK widget and the render window
    vtkWidget = new QVTKOpenGLNativeWidget(this);
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    vtkWidget->setRenderWindow(renderWindow);

    // Create a renderer and add it to the window
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Simple example: sphere actor
    vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->SetRadius(0.5);
    sphere->SetThetaResolution(32);
    sphere->SetPhiResolution(32);
    sphere->Update();

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(sphere->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    renderer->AddActor(actor);
    renderer->SetBackground(0.12, 0.16, 0.2);

    // Place the VTK widget into the main window as central widget
    setCentralWidget(vtkWidget);

    renderWindow->Render();
}
