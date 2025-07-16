#include "../header/ImageCapture.h"


void saveScreenShot(int width, int height, const char *filename) {
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
