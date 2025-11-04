#include "ImageCapture.h"
#include <iostream>
#include <fstream>
#include <SOIL/SOIL.h>
#include <GL/glew.h>


ImageCapture::ImageCapture() = default;

ImageCapture::~ImageCapture() = default;

void ImageCapture::saveScreenShot(int width, int height, const char *filename) {
    std::vector<unsigned char> pixels(width*height*3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    std::vector<unsigned char> flipped(pixels.size());
    for (int y = 0; y < height; ++y) {
        std::copy(
            pixels.begin() + y * width * 3,
            pixels.begin() + (y + 1) * width * 3,
            flipped.begin() + (height - 1 - y) * width * 3
        );
    }

    int result = SOIL_save_image(filename,SOIL_SAVE_TYPE_TGA, width, height, 3, flipped.data());
    if (result == 0) {
        std::cerr << "SOIL failed to save screenshot: " << SOIL_last_result() << std::endl;
    } else {
        std::cout << "Screenshot saved to " << filename << std::endl;
    }
}

void ImageCapture::saveGreyImage(int width, int height,const std::vector <unsigned char> & gray, const char *filename) {
    std::vector<unsigned char> flipped_vector(gray.size());

    for (int y = 0; y < height; ++y) {
        std::copy(
            gray.begin() + y * width,
            gray.begin() + (y + 1) * width,
            flipped_vector.begin() + (height - 1 - y) * width
            );
    }
    int result = SOIL_save_image(filename, SOIL_SAVE_TYPE_TGA, width, height, 1, flipped_vector.data());
    if (result == 0) {
        std::cerr << "SOIL failed to save GrayImage screenshot: " << SOIL_last_result() << std::endl;
    }
    else {
        std::cout << "GrayImage saved to " << filename << std::endl;
    }

}


