#pragma once

#include "pch.h"

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
    MainWindow(QWidget* parent = nullptr);

private slots:

    void SelectSkyboxFolder();
    void SelectDiskTexture();
    void BlackholeCheckboxUpdate();
    void AccretionDiskCheckboxUpdate();
    void Render();

private:
    void SetupUI();
    void SetupAction();

private:
    Ui::MainWindow* ui_;
    std::shared_ptr<uint8_t[]> img_;
    QGraphicsScene* scene_;
};