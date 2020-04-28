#include "pch.h"

#include "image_generator.h"
#include "main_window.h"
#include "ui_main_window.h"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    img_generator_.reset(new ImageGenerator());
    vid_generator_.reset(new VideoGenerator());
    vid_generator_->SetImageGenerator(img_generator_);
    ui_->setupUi(this);
    SetupUI();
    SetupAction();
}

void MainWindow::BlackholeCheckboxUpdate()
{
    if (!ui_->blackhole_checkbox->isChecked())
    {
        ui_->accretion_disk_checkbox->setEnabled(false);
        ui_->disk_browser_lineedit->setEnabled(false);
        ui_->disk_browser_button->setEnabled(false);
    }
    else
    {
        ui_->accretion_disk_checkbox->setEnabled(true);
        ui_->disk_browser_lineedit->setEnabled(true);
        ui_->disk_browser_button->setEnabled(true);
    }
}

void MainWindow::BloomCheckboxUpdate()
{
    if (img_generator_->IsRendering())
        return;

    if (ui_->bloom_checkbox->isChecked())
    {
        img_generator_->Bloom();
        img_ = img_generator_->ResultBuffer();
    }
    else
    {
        img_ = img_generator_->ColorBuffer();
    }
    pixmap_item_->setPixmap(QPixmap::fromImage(
        QImage(img_->data, img_->size().width, img_->size().height, QImage::Format_RGB888).rgbSwapped()));
}

void MainWindow::AccretionDiskCheckboxUpdate()
{
    if (!ui_->accretion_disk_checkbox->isChecked())
    {
        ui_->disk_browser_lineedit->setEnabled(false);
        ui_->disk_browser_button->setEnabled(false);
        ui_->disk_radius_widget->setEnabled(false);
    }
    else
    {
        ui_->disk_browser_lineedit->setEnabled(true);
        ui_->disk_browser_button->setEnabled(true);
        ui_->disk_radius_widget->setEnabled(true);
    }
}
void MainWindow::SelectSkyboxFolder()
{
    QString directory =
        QFileDialog::getExistingDirectory(this, tr("Select Skybox Folder"), QDir::currentPath() + "resources/skybox");

    if (!directory.isEmpty())
    {
        ui_->skybox_path_lineedit->setText(directory);
        skybox_need_load_ = true;
    }
}


void MainWindow::SelectDiskTexture()
{
    QString directory =
        QFileDialog::getOpenFileName(this, tr("Select Disk Texture"), QDir::currentPath() + "resources/disk");

    if (!directory.isEmpty())
    {
        ui_->disk_browser_lineedit->setText(directory);
    }
}

void MainWindow::Abort()
{
    img_generator_->Abort();
    vid_generator_->Abort();
    rendering_ = false;
}

void MainWindow::RenderImage()
{
    uint32_t width  = ui_->width_lineedit->text().toInt();
    uint32_t height = ui_->height_lineedit->text().toInt();
    int samples     = ui_->samples_box->currentText().toInt();
    std::filesystem::path skybox_folder_path(ui_->skybox_path_lineedit->text().toStdString());
    glm::dvec3 camera_pos(ui_->cam_pox_x_lineedit->text().toDouble(), ui_->cam_pox_y_lineedit->text().toDouble(),
        ui_->cam_pox_z_lineedit->text().toDouble());
    glm::dvec3 camera_lookat(ui_->cam_lookat_x_lineedit->text().toDouble(),
        ui_->cam_lookat_y_lineedit->text().toDouble(), ui_->cam_lookat_z_lineedit->text().toDouble());
    Camera camera(camera_pos, camera_lookat);

    if (skybox_need_load_)
    {
        img_generator_->LoadSkybox(skybox_folder_path);
        skybox_need_load_ = false;
    }
    img_generator_->SetCamera(camera);
    img_generator_->SetSamples(samples);
    img_generator_->SetSize(width, height);
    img_ = img_generator_->ColorBuffer();


    std::shared_ptr<Blackhole> blackhole;
    if (ui_->blackhole_checkbox->isChecked())
    {
        double disk_inner, disk_outer;
        if (ui_->accretion_disk_checkbox->isChecked())
        {
            disk_inner = ui_->disk_inner_lineedit->text().toDouble();
            disk_outer = ui_->disk_outer_lineedit->text().toDouble();
        }
        else
        {
            disk_inner = 1;
            disk_outer = 1;
        }
        std::filesystem::path disk_texture_path = ui_->disk_browser_lineedit->text().toStdString();
        blackhole = std::make_shared<Blackhole>(glm::dvec3(0, 0, 0), disk_inner, disk_outer, disk_texture_path);
        img_generator_->SetBlackhole(*blackhole);
    }
    else
    {
        img_generator_->RemoveBlackhole();
    }

    img_generator_->SetThreads(ui_->threads_box->currentText().toInt());

    QImage::Format format = QImage::Format_RGB888;

    std::atomic<bool> finished = false;

    auto generate_thread = std::thread([&] {
        img_generator_->Generate();
        finished = true;
    });

    scene_->setSceneRect(0, 0, img_->size().width, img_->size().height);
    while (!finished)
    {
        std::this_thread::sleep_for(0.1s);
        pixmap_item_->setPixmap(
            QPixmap::fromImage(QImage(img_->data, img_->size().width, img_->size().height, format).rgbSwapped()));
        ui_->graphicsView->update();
        scene_->update();
        ui_->graphicsView->fitInView(pixmap_item_, Qt::KeepAspectRatio);

        qApp->processEvents();
    }
    generate_thread.join();

    pixmap_item_->setPixmap(
        QPixmap::fromImage(QImage(img_->data, img_->size().width, img_->size().height, format).rgbSwapped()));
}

void MainWindow::RenderVideo()
{


    uint32_t width  = ui_->width_lineedit->text().toInt();
    uint32_t height = ui_->height_lineedit->text().toInt();
    int samples     = ui_->samples_box->currentText().toInt();
    std::filesystem::path skybox_folder_path(ui_->skybox_path_lineedit->text().toStdString());
    Camera camera;

    if (skybox_need_load_)
    {
        img_generator_->LoadSkybox(skybox_folder_path);
        skybox_need_load_ = false;
    }
    img_generator_->SetCamera(camera);
    img_generator_->SetSamples(samples);
    img_generator_->SetSize(width, height);
    img_ = img_generator_->ColorBuffer();


    std::shared_ptr<Blackhole> blackhole;
    if (ui_->blackhole_checkbox->isChecked())
    {
        double disk_inner, disk_outer;
        if (ui_->accretion_disk_checkbox->isChecked())
        {
            disk_inner = ui_->disk_inner_lineedit->text().toDouble();
            disk_outer = ui_->disk_outer_lineedit->text().toDouble();
        }
        else
        {
            disk_inner = 1;
            disk_outer = 1;
        }
        std::filesystem::path disk_texture_path = ui_->disk_browser_lineedit->text().toStdString();
        blackhole = std::make_shared<Blackhole>(glm::dvec3(0, 0, 0), disk_inner, disk_outer, disk_texture_path);
        img_generator_->SetBlackhole(*blackhole);
    }
    else
    {
        img_generator_->RemoveBlackhole();
    }

    img_generator_->SetThreads(ui_->threads_box->currentText().toInt());

    QImage::Format format = QImage::Format_RGB888;

    std::atomic<bool> finished = false;

    std::filesystem::path position_file_path(ui_->pos_browser_lineedit->text().toStdString());
    vid_generator_->LoadPositions(position_file_path);
    vid_generator_->SetBloom(ui_->bloom_checkbox->isChecked());
    vid_generator_->SetOutputFilePath("video.mp4");

    auto generate_thread = std::thread([&] {
        vid_generator_->GenerateAndSave();
        finished = true;
    });

    scene_->setSceneRect(0, 0, img_->size().width, img_->size().height);
    while (!finished)
    {
        std::this_thread::sleep_for(0.1s);
        pixmap_item_->setPixmap(
            QPixmap::fromImage(QImage(img_->data, img_->size().width, img_->size().height, format).rgbSwapped()));
        ui_->graphicsView->update();
        scene_->update();
        ui_->graphicsView->fitInView(pixmap_item_, Qt::KeepAspectRatio);

        qApp->processEvents();
    }
    generate_thread.join();

    pixmap_item_->setPixmap(
        QPixmap::fromImage(QImage(img_->data, img_->size().width, img_->size().height, format).rgbSwapped()));
}


void MainWindow::RenderOrAbort()
{

    ui_->render_button->setText("Abort");
    if (rendering_)
    {
        Abort();
        ui_->render_button->setText("Render");
        return;
    }

    mode_ = ui_->image_mode_button->isChecked() ? kImageMode : kVideoMode;

    auto start_time = std::chrono::high_resolution_clock::now();

    switch (mode_)
    {
    case kImageMode:
        rendering_ = true;
        RenderImage();
        break;
    case kVideoMode:
        rendering_ = true;
        RenderVideo();
        break;
    default:
        throw std::runtime_error("unknown mode");
        break;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
    statusBar()->showMessage("Render Time: " + QString::number(duration) + " seconds");
    ui_->render_button->setText("Render");
    rendering_ = false;
}


void MainWindow::SetupUI()
{
    QDoubleValidator* pos_validator = new QDoubleValidator(-10000, 10000, 2, this);
    pos_validator->setNotation(QDoubleValidator::StandardNotation);
    ui_->cam_pox_x_lineedit->setValidator(pos_validator);
    ui_->cam_pox_y_lineedit->setValidator(pos_validator);
    ui_->cam_pox_z_lineedit->setValidator(pos_validator);
    ui_->cam_lookat_x_lineedit->setValidator(pos_validator);
    ui_->cam_lookat_y_lineedit->setValidator(pos_validator);
    ui_->cam_lookat_z_lineedit->setValidator(pos_validator);
    ui_->width_lineedit->setValidator(new QIntValidator(1, 4096, this));
    ui_->height_lineedit->setValidator(new QIntValidator(1, 4096, this));
    scene_       = new QGraphicsScene();
    pixmap_item_ = new QGraphicsPixmapItem();
    scene_->addItem(pixmap_item_);
    ui_->graphicsView->setScene(scene_);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    // Your code here.
    ui_->graphicsView->fitInView(pixmap_item_, Qt::KeepAspectRatio);
}

void MainWindow::WidthUpdate()
{
    const QValidator* validator = ui_->width_lineedit->validator();

    int pos    = 0;
    auto text  = ui_->width_lineedit->text();
    auto state = validator->validate(text, pos);
    if (state != QValidator::Acceptable)
    {
        ui_->width_lineedit->setText(ui_->height_lineedit->text());
        ui_->error_label->setText("Resolution ranges from 1 to 4096");
    }
    else
    {
        ui_->height_lineedit->setText(ui_->width_lineedit->text());
    }
}

void MainWindow::SkyboxPathUpdate()
{
    skybox_need_load_ = true;
}

void MainWindow::SaveToDisk()
{
    QString filter = "PNG (*.png)";

    QString directory = QFileDialog::getSaveFileName(this, tr("Save as"), QDir::currentPath(), filter, &filter);
    if (ui_->bloom_checkbox->isChecked())
    {
        cv::imwrite(directory.toStdString(), *(img_generator_->ResultBuffer()));
    }
    else
    {
        cv::imwrite(directory.toStdString(), *(img_generator_->ColorBuffer()));
    }
}

void MainWindow::SelectPositionFile()
{
    QString directory =
        QFileDialog::getOpenFileName(this, tr("Select Position file"), QDir::currentPath() + "resources/position");

    if (!directory.isEmpty())
    {
        ui_->pos_browser_lineedit->setText(directory);
    }
}

void MainWindow::SetupAction()
{
    connect(ui_->skybox_browser_button, SIGNAL(clicked()), SLOT(SelectSkyboxFolder()));
    connect(ui_->disk_browser_button, SIGNAL(clicked()), SLOT(SelectDiskTexture()));
    connect(ui_->pos_browser_button, SIGNAL(clicked()), SLOT(SelectPositionFile()));
    connect(ui_->render_button, SIGNAL(clicked()), SLOT(RenderOrAbort()));
    connect(ui_->blackhole_checkbox, SIGNAL(stateChanged(int)), SLOT(BlackholeCheckboxUpdate()));
    connect(ui_->accretion_disk_checkbox, SIGNAL(stateChanged(int)), SLOT(AccretionDiskCheckboxUpdate()));
    connect(ui_->width_lineedit, SIGNAL(editingFinished()), SLOT(WidthUpdate()));
    connect(ui_->bloom_checkbox, SIGNAL(stateChanged(int)), SLOT(BloomCheckboxUpdate()));
    connect(ui_->skybox_path_lineedit, SIGNAL(editingFinished()), SLOT(SkyboxPathUpdate()));
    connect(ui_->actionSave, SIGNAL(triggered()), SLOT(SaveToDisk()));
}
