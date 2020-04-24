#pragma once

#include "pch.h"

class Camera
{
public:
    explicit Camera(glm::dvec3 position, glm::dvec3 look_at, glm::dvec3 world_up = glm::dvec3(0, 1, 0), double fov = 90)
        : position_(position), world_up_(world_up)
    {
        LookAt(look_at);
        fov_ = glm::radians(fov);
    }

    void SetPosition(glm::dvec3 pos) { position_ = pos; }

    // keep front vector invariant
    void SetWorldUp(glm::dvec3 world_up)
    {
        this->world_up_ = glm::normalize(world_up);
        UpdateVectors();
    }

    void LookAt(glm::dvec3 point)
    {
        front_ = glm::normalize(point - position_);
        UpdateVectors();
    }

    glm::dvec3 GetTexCoord(int row, int col, int width, int height)
    {
        glm::dvec3 tex_coord;

        double center_x = double(width) / 2;
        double center_y = double(height) / 2;
        double bend_x   = (double(col) - center_x) / center_x * (fov_ / 2);
        double bend_y   = -(double(row) - center_y) / center_y * (fov_ / 2);

        double up_length    = glm::length(front_) * tan(bend_y);
        double right_length = glm::length(front_) * tan(bend_x);
        tex_coord           = front_ + up_ * up_length;
        tex_coord           = tex_coord + right_ * right_length;

        return tex_coord;
    }

    inline double FoV() { return fov_; }
    inline glm::dvec3 Front() { return front_; }
    inline glm::dvec3 Up() { return up_; }
    inline glm::dvec3 Right() { return right_; }
    inline glm::dvec3 Position() { return position_; }

private:
    glm::dvec3 position_;
    glm::dvec3 front_;
    glm::dvec3 up_;
    glm::dvec3 right_;
    glm::dvec3 world_up_;
    double fov_;

    void UpdateVectors()
    {
        right_ = glm::normalize(glm::cross(front_, world_up_));
        up_    = glm::normalize(glm::cross(right_, front_));
    }
};