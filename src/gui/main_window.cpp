#include "pch.h"

#include "image_generator.h"
#include "main_window.h"
#include "ui_main_window.h"


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    ui_->cam_pox_x_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->cam_pox_y_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->cam_pox_z_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->cam_lookat_x_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->cam_lookat_y_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->cam_lookat_z_lineedit->setValidator(new QDoubleValidator(1, 100, 2, this));
    ui_->width_lineedit->setValidator(new QIntValidator(1, 4096, this));
    ui_->width_lineedit->setValidator(new QIntValidator(1, 4096, this));
    scene_ = new QGraphicsScene();
    ui_->graphicsView->setScene(scene_);
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
void MainWindow::AccretionDiskCheckboxUpdate()
{
    if (!ui_->accretion_disk_checkbox->isChecked())
    {
        ui_->disk_browser_lineedit->setEnabled(false);
        ui_->disk_browser_button->setEnabled(false);
    }
    else
    {
        ui_->disk_browser_lineedit->setEnabled(true);
        ui_->disk_browser_button->setEnabled(true);
    }
}
void MainWindow::SelectSkyboxFolder()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select Skybox Folder"), QDir::currentPath());

    if (!directory.isEmpty())
    {
        ui_->skybox_path_lineedit->setText(directory);
    }
}


void MainWindow::SelectDiskTexture()
{
    QString directory = QFileDialog::getOpenFileName(this, tr("Select Disk Texture"), QDir::currentPath());

    if (!directory.isEmpty())
    {
        ui_->disk_browser_lineedit->setText(directory);
    }
}

void MainWindow::Render()
{
    std::filesystem::path skybox_folder_path(ui_->skybox_path_lineedit->text().toStdString());
    glm::dvec3 camera_pos(ui_->cam_pox_x_lineedit->text().toDouble(), ui_->cam_pox_x_lineedit->text().toDouble(),
        ui_->cam_pox_x_lineedit->text().toDouble());
    glm::dvec3 camera_lookat(ui_->cam_lookat_x_lineedit->text().toDouble(),
        ui_->cam_lookat_y_lineedit->text().toDouble(), ui_->cam_lookat_z_lineedit->text().toDouble());
    Camera camera(camera_pos, camera_lookat);
    ImageGenerator img_generator(skybox_folder_path, camera, std::pow(2, ui_->samples_box->currentIndex()),
        ui_->width_lineedit->text().toDouble(), ui_->height_lineedit->text().toDouble());
    img_generator.Generate();
    Image* image = img_generator.GetImage();
    img_.reset(new uint8_t[image->Height() * image->Width() * 4]);
    for (int row = 0; row < image->Height(); ++row)
    {
        for (int col = 0; col < image->Width(); ++col)
        {
            glm::dvec4 hdr_color                         = image->Get(col, row);
            img_[row * image->Width() * 4 + col * 4 + 0] = std::clamp(hdr_color.r * 255, 0.0, 255.0);
            img_[row * image->Width() * 4 + col * 4 + 1] = std::clamp(hdr_color.g * 255, 0.0, 255.0);
            img_[row * image->Width() * 4 + col * 4 + 2] = std::clamp(hdr_color.b * 255, 0.0, 255.0);
            img_[row * image->Width() * 4 + col * 4 + 3] = 255;
        }
    }
    QImage qimage(img_.get(), image->Width(), image->Height(), QImage::Format_RGBA8888);
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(QPixmap::fromImage(qimage));

    scene_->addItem(item);
    scene_->update();

}


void MainWindow::SetupUI()
{
}

void MainWindow::SetupAction()
{
    connect(ui_->skybox_browser_button, SIGNAL(clicked()), SLOT(SelectSkyboxFolder()));
    connect(ui_->disk_browser_button, SIGNAL(clicked()), SLOT(SelectDiskTexture()));
    connect(ui_->render_button, SIGNAL(clicked()), SLOT(Render()));
    connect(ui_->blackhole_checkbox, SIGNAL(stateChanged(int)), SLOT(BlackholeCheckboxUpdate()));
    connect(ui_->accretion_disk_checkbox, SIGNAL(stateChanged(int)), SLOT(AccretionDiskCheckboxUpdate()));
}
