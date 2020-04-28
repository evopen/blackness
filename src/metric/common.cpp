#include "common.h"


cv::Vec3b metric::common::SkyboxSampler(const glm::dvec3& tex_coord, const Skybox& skybox)
{
    std::vector<double> temp_vector = {tex_coord.x, tex_coord.y, tex_coord.z};
    int max_abs_index               = std::distance(
        temp_vector.begin(), std::max_element(temp_vector.begin(), temp_vector.end(), util::AbsCompare<double>));

    std::shared_ptr<cv::Mat> image;

    switch (max_abs_index)
    {
    case 2:
        if (tex_coord[max_abs_index] > 0)
            image = skybox.back;
        else
            image = skybox.front;
        break;
    case 1:
        if (tex_coord[max_abs_index] > 0)
            image = skybox.top;
        else
            image = skybox.bottom;
        break;
    case 0:
        if (tex_coord[max_abs_index] > 0)
            image = skybox.right;
        else
            image = skybox.left;
        break;
    default:
        throw std::runtime_error("error");
    }

    double scale = 1 / tex_coord[max_abs_index];

    // delete index that I don't need
    temp_vector.erase(temp_vector.begin() + max_abs_index);

    glm::dvec2 coord_2d(std::min(1.0, std::max(0.0, (temp_vector[0] * scale + 1) / 2)),
        std::min(1.0, std::max(0.0, (temp_vector[1] * scale + 1) / 2)));

    if (coord_2d.x > 1 || coord_2d.y > 1 || coord_2d.x < 0 || coord_2d.y < 0)
        throw std::runtime_error("2d coord out of range 0 to 1");


    int max_row_col = skybox.front->size().height - 1;

    int row = std::lround(coord_2d[1] * max_row_col);
    int col = std::lround(coord_2d[0] * max_row_col);
    return image->at<cv::Vec3b>(row, col);
}
