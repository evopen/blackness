#pragma once

#include "movie_writer.h"

#include <boost/math/tools/roots.hpp>
#include <cxxopts.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <gsl/gsl_integration.h>
#include <omp.h>
#include <opencv2/opencv.hpp>

#include <QApplication>
#include <QCheckBox>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QGraphicsPixmapItem>
#include <QPixmap>

#include <array>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <random>
#include <thread>
#include <vector>