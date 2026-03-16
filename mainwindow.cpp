#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QSlider>
#include <QSurfaceFormat>
#include <QTimer>
#include <QStringList>
#include <QVBoxLayout>
#include <vtkActor.h>
#include <vtkAxesActor.h>
#include <vtkOBJReader.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSphereSource.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  setupVTK();
  setupControlPanel();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupVTK()
{
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

  vtkWidget = new QVTKOpenGLNativeWidget(this);
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
      qDebug() << "Bounds:" << bounds[0] << bounds[1] << bounds[2] << bounds[3]
               << bounds[4] << bounds[5];
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

  QTimer::singleShot(0, this, [this]()
                     {
    vtkRenderWindowInteractor *interactor = vtkWidget->interactor();
    if (!interactor) {
      qWarning() << "QVTK widget has no interactor; orientation marker will be "
                    "disabled.";
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
    renderWindow->Render(); });

  renderer->SetBackground(0.12, 0.16, 0.2);
  renderer->ResetCamera();
  renderer->ResetCameraClippingRange();

  renderWindow->Render();
}

void MainWindow::setupControlPanel()
{
  // Build the top-level central widget with a horizontal layout:
  //   [vtkWidget (stretch)] | [control panel (fixed width)]
  QWidget *centralWidget = new QWidget(this);
  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  mainLayout->addWidget(vtkWidget, 1);

  // --- Control panel ---
  QGroupBox *panel = new QGroupBox("Orientation", centralWidget);
  panel->setFixedWidth(200);
  QVBoxLayout *panelLayout = new QVBoxLayout(panel);
  panelLayout->setSpacing(8);

  auto makeSliderRow = [&](const QString &label, QSlider *&slider,
                           QLabel *&valueLabel)
  {
    panelLayout->addWidget(new QLabel(label, panel));

    slider = new QSlider(Qt::Horizontal, panel);
    slider->setRange(-180, 180);
    slider->setValue(0);
    slider->setTickInterval(45);
    slider->setTickPosition(QSlider::TicksBelow);
    panelLayout->addWidget(slider);

    valueLabel = new QLabel("0°", panel);
    valueLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(valueLabel);
  };

  makeSliderRow("Roll (X)", rollSlider, rollValueLabel);
  makeSliderRow("Pitch (Y)", pitchSlider, pitchValueLabel);
  makeSliderRow("Yaw (Z)", yawSlider, yawValueLabel);

  panelLayout->addStretch();
  mainLayout->addWidget(panel);

  setCentralWidget(centralWidget);

  connect(rollSlider,  &QSlider::valueChanged, this, &MainWindow::onRollChanged);
  connect(pitchSlider, &QSlider::valueChanged, this, &MainWindow::onPitchChanged);
  connect(yawSlider,   &QSlider::valueChanged, this, &MainWindow::onYawChanged);
}

void MainWindow::onRollChanged(int value)
{
  rollValueLabel->setText(QString::number(value) + "°");
  updateOrientation();
}

void MainWindow::onPitchChanged(int value)
{
  pitchValueLabel->setText(QString::number(value) + "°");
  updateOrientation();
}

void MainWindow::onYawChanged(int value)
{
  yawValueLabel->setText(QString::number(value) + "°");
  updateOrientation();
}

void MainWindow::updateOrientation()
{
  if (!satelliteActor)
    return;

  // VTK SetOrientation(rx, ry, rz) applies rotations in X→Y→Z order
  satelliteActor->SetOrientation(
      rollSlider->value(),   // X (Roll)
      pitchSlider->value(),  // Y (Pitch)
      yawSlider->value()     // Z (Yaw)
  );

  renderWindow->Render();
}
