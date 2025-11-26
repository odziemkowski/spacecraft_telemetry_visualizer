#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <vtkActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>
#include <vtkOBJReader.h>
#include <vtkPolyData.h>
#include <QSurfaceFormat>
#include <QDebug>
#include <QIcon>
#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <vtkAxesActor.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Create icon with multiple sizes for better scaling
    // QIcon icon;
    //
    //// Try embedded Qt resources first
    // QStringList resourceSizes = {"16", "32", "48", "128"};
    // bool foundResource = false;
    //
    // for (const QString &size : resourceSizes)
    //{
    //    QString resourcePath = QString(":/icons/icon-%1.png").arg(size);
    //    if (QFile::exists(resourcePath))
    //    {
    //        icon.addFile(resourcePath, QSize(size.toInt(), size.toInt()));
    //        foundResource = true;
    //    }
    //}
    //
    // if (foundResource)
    //{
    //    setWindowIcon(icon);
    //}
    // else
    //{
    //    // Fall back to filesystem locations
    //    QStringList basePaths = {
    //        QCoreApplication::applicationDirPath() + "/icons/",
    //        QCoreApplication::applicationDirPath() + "/../icons/",
    //        QCoreApplication::applicationDirPath() + "/../../icons/",
    //        "icons/"};
    //
    //    for (const QString &basePath : basePaths)
    //    {
    //        bool foundAny = false;
    //        for (const QString &size : resourceSizes)
    //        {
    //            QString iconPath = basePath + QString("icon-%1.png").arg(size);
    //            if (QFile::exists(iconPath))
    //            {
    //                icon.addFile(iconPath, QSize(size.toInt(), size.toInt()));
    //                foundAny = true;
    //            }
    //        }
    //        if (foundAny)
    //        {
    //            setWindowIcon(icon);
    //            break;
    //        }
    //    }
    //}

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

    // Disable multisampling on the render window before attaching it to the
    // widget — doing this early helps avoid mismatches between widget FBOs
    // and the render window that can lead to glBlitFramebuffer errors.
    renderWindow->SetMultiSamples(0);
    vtkWidget->setRenderWindow(renderWindow);

    // Create a renderer and add it to the window
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    // Try to load an OBJ model from common `models/` locations. Fall back to a sphere if not found.
    QStringList modelSearchPaths = {
        QCoreApplication::applicationDirPath() + "/models/satellite.obj",
        "models/satellite.obj"};

    QString modelPath;
    for (const QString &p : modelSearchPaths)
    {
        qInfo() << "Model current search path:" << p;
        if (QFile::exists(p))
        {
            modelPath = p;
            break;
        }
    }

    if (!modelPath.isEmpty())
    {
        qDebug() << "Found model file:" << modelPath;

        vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
        reader->SetFileName(modelPath.toStdString().c_str());
        reader->Update();

        vtkPolyData *poly = vtkPolyData::SafeDownCast(reader->GetOutput());
        if (poly)
        {
            double bounds[6];
            poly->GetBounds(bounds);
            qDebug() << "OBJ loaded: points=" << poly->GetNumberOfPoints()
                     << "cells=" << poly->GetNumberOfCells();
            qDebug() << "Bounds:" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
        }
        else
        {
            qDebug() << "Warning: reader returned no polydata output.";
        }

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        renderer->AddActor(actor);
    }
    else
    {
        qDebug() << "No OBJ found; using fallback sphere.";
        // Simple example: sphere actor (fallback)
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(0.5);
        sphere->SetThetaResolution(32);
        sphere->SetPhiResolution(32);
        sphere->Update();

        vtkPolyData *poly = vtkPolyData::SafeDownCast(sphere->GetOutput());
        if (poly)
        {
            qDebug() << "Sphere geometry: points=" << poly->GetNumberOfPoints()
                     << "cells=" << poly->GetNumberOfCells();
        }

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphere->GetOutputPort());

        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        renderer->AddActor(actor);
    }

    // We'll create the axes and the orientation marker after the widget
    // and its interactor are fully initialized by the Qt event loop. This
    // avoids timing issues where `vtkWidget->interactor()` can be null
    // during setup, and also ensures the orientation widget object is
    // kept alive as a member (`orientationWidget`).
    QTimer::singleShot(0, this, [this]()
                       {
        vtkRenderWindowInteractor *interactor = vtkWidget->interactor();
        if (!interactor)
        {
            qWarning() << "QVTK widget has no interactor; orientation marker will be disabled.";
            return;
        }

        vtkSmartPointer<vtkAxesActor> axes = vtkSmartPointer<vtkAxesActor>::New();
        axes->SetTotalLength(1.0, 1.0, 1.0);
        axes->SetShaftTypeToCylinder();
        axes->SetNormalizedShaftLength(0.8, 0.8, 0.8);
        axes->SetNormalizedTipLength(0.2, 0.2, 0.2);

        orientationWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
        orientationWidget->SetInteractor(interactor);
        orientationWidget->SetOrientationMarker(axes);
        orientationWidget->SetViewport(0.0, 0.0, 0.2, 0.2);
        orientationWidget->SetEnabled(1);
        orientationWidget->InteractiveOff(); });

    renderer->SetBackground(0.12, 0.16, 0.2);

    // Ensure the camera frames the scene if any actor was added (e.g. the OBJ)
    if (renderer->GetActors()->GetNumberOfItems() > 0)
    {
        renderer->ResetCamera();
        renderer->ResetCameraClippingRange();
    }

    // Place the VTK widget into the main window as central widget
    setCentralWidget(vtkWidget);

    // Final render to push initial frame to the widget
    renderWindow->Render();
}
