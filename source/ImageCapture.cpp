#include "ImageCapture.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <include/stb-master/stb_image_write.h>

ImageCapture::ImageCapture() = default;

ImageCapture::~ImageCapture() = default;

void ImageCapture::saveScreenShot(int width, int height, const char* filename) {
    std::vector<unsigned char> pixels(width * height * 3);
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


    if (stbi_write_png(filename, width, height, 3, flipped.data(),0)) {
        std::cout << "Screenshot saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save screenshot: " << filename << std::endl;
    }
}

void ImageCapture::saveGreyImage(int width, int height, const std::vector<unsigned char>& gray, const char* filename) {
    if (stbi_write_png(filename, width, height, 1, gray.data(), width)) {
        std::cout << "Gray image saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save gray image: " << filename << std::endl;
    }
}
void ImageCapture::saveColorImage(int width, int height, const std::vector<unsigned char>& rgb, const char* filename) {
    if (stbi_write_png(filename, width, height, 3, rgb.data(), width * 3)) {
        std::cout << "Color image saved to " << filename << std::endl;
    } else {
        std::cerr << "Failed to save color image: " << filename << std::endl;
    }
}




