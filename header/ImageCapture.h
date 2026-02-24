#pragma once


#include <vector>
class ImageCapture {
public:
   ImageCapture();
    ~ImageCapture();
    static void saveScreenShot(int width, int height, const char *filename);
    static void saveGreyImage(int width, int height,const std::vector <unsigned char> & gray, const char *filename);
    static void saveColorImage(int width, int height, const std::vector<unsigned char>& rgb, const char* filename);

};





