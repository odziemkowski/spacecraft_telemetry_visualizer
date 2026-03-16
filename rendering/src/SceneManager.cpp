#include "SceneManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include <QTimer>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkAxesActor.h>
#include <vtkOBJReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>

SceneManager::SceneManager(QWidget *parent)
    : vtkWidget(new QVTKOpenGLNativeWidget(parent))
{
}

void SceneManager::setup()
{
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    renderWindow->SetMultiSamples(0);
    vtkWidget->setRenderWindow(renderWindow);

    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    QStringList modelSearchPaths = {QCoreApplication::applicationDirPath() +
                                        "/models/satellite.obj",
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

    satelliteActor = vtkSmartPointer<vtkActor>::New();

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
            qDebug() << "Bounds:" << bounds[0] << bounds[1] << bounds[2]
                     << bounds[3] << bounds[4] << bounds[5];
        }
        else
        {
            qDebug() << "Warning: reader returned no polydata output.";
        }

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(reader->GetOutputPort());
        satelliteActor->SetMapper(mapper);
    }
    else
    {
        qDebug() << "No OBJ found; using fallback sphere.";
        vtkSmartPointer<vtkSphereSource> sphere =
            vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetRadius(0.5);
        sphere->SetThetaResolution(32);
        sphere->SetPhiResolution(32);
        sphere->Update();

        vtkSmartPointer<vtkPolyDataMapper> mapper =
            vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphere->GetOutputPort());
        satelliteActor->SetMapper(mapper);
    }

    renderer->AddActor(satelliteActor);

    QTimer::singleShot(0, vtkWidget, [this]()
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
        orientationWidget->InteractiveOff();
        renderWindow->Render();
    });

    renderer->SetBackground(0.12, 0.16, 0.2);
    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();

    renderWindow->Render();
}

void SceneManager::updateOrientation(int roll, int pitch, int yaw)
{
    if (!satelliteActor)
        return;

    // VTK SetOrientation(rx, ry, rz) applies rotations in X→Y→Z order
    satelliteActor->SetOrientation(
        static_cast<double>(roll),   // X (Roll)
        static_cast<double>(pitch),  // Y (Pitch)
        static_cast<double>(yaw)     // Z (Yaw)
    );

    renderWindow->Render();
}

QWidget *SceneManager::widget()
{
    return vtkWidget;
}
