#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QSlider>
#include <QVTKOpenGLNativeWidget.h>
#include <vtkActor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
class vtkOrientationMarkerWidget;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRollChanged(int value);
    void onPitchChanged(int value);
    void onYawChanged(int value);

private:
    Ui::MainWindow *ui;
    QVTKOpenGLNativeWidget *vtkWidget;
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;
    vtkSmartPointer<vtkOrientationMarkerWidget> orientationWidget;
    vtkSmartPointer<vtkActor> satelliteActor;

    QSlider *rollSlider;
    QSlider *pitchSlider;
    QSlider *yawSlider;
    QLabel *rollValueLabel;
    QLabel *pitchValueLabel;
    QLabel *yawValueLabel;

    void setupVTK();
    void setupControlPanel();
    void updateOrientation();
};
#endif // MAINWINDOW_H
