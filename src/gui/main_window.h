#pragma once

#include "pch.h"
#include "image_generator.h"

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
    void BloomCheckboxUpdate();
    void AccretionDiskCheckboxUpdate();
    void RenderOrAbort();
    void WidthUpdate();
    void SkyboxPathUpdate();
    void SaveToDisk();

private:
    void SetupUI();
    void SetupAction();
    void MainWindow::resizeEvent(QResizeEvent* event);

private:
    Ui::MainWindow* ui_;
    std::shared_ptr<const cv::Mat> img_;
    QGraphicsScene* scene_;
    QGraphicsPixmapItem* pixmap_item_;
    std::unique_ptr<ImageGenerator> img_generator_;
    bool skybox_need_load_ = true;
};