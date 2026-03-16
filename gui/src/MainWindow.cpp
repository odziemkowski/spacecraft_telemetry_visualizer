#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "SceneManager.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    sceneManager = std::make_unique<SceneManager>(this);
    sceneManager->setup();
    setupControlPanel();
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setupControlPanel()
{
    // Build the top-level central widget with a horizontal layout:
    //   [VTK viewport (stretch)] | [control panel (fixed width)]
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    mainLayout->addWidget(sceneManager->widget(), 1);

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
    applyOrientation();
}

void MainWindow::onPitchChanged(int value)
{
    pitchValueLabel->setText(QString::number(value) + "°");
    applyOrientation();
}

void MainWindow::onYawChanged(int value)
{
    yawValueLabel->setText(QString::number(value) + "°");
    applyOrientation();
}

void MainWindow::applyOrientation()
{
    sceneManager->updateOrientation(
        rollSlider->value(),
        pitchSlider->value(),
        yawSlider->value()
    );
}
