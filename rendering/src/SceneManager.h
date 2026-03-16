#pragma once

#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

class QWidget;
class QVTKOpenGLNativeWidget;

class SceneManager
{
public:
    explicit SceneManager(QWidget *parent = nullptr);

    void setup();
    void updateOrientation(int roll, int pitch, int yaw);
    QWidget *widget();

private:
    QVTKOpenGLNativeWidget *vtkWidget;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationWidget;
    vtkSmartPointer<vtkActor> satelliteActor;
};
