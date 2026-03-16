#pragma once

#include <QLabel>
#include <QMainWindow>
#include <QSlider>
#include <memory>

class SceneManager;

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
    std::unique_ptr<SceneManager> sceneManager;

    QSlider *rollSlider;
    QSlider *pitchSlider;
    QSlider *yawSlider;
    QLabel *rollValueLabel;
    QLabel *pitchValueLabel;
    QLabel *yawValueLabel;

    void setupControlPanel();
    void applyOrientation();
};
