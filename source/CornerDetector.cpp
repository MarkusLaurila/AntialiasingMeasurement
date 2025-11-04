#include "CornerDetector.h"
#include <iostream>
#include <algorithm>
#include "ImageCapture.h"

CornerDetector::CornerDetector(int width, int height)
    : width(width), height(height) ,fftPlan(nullptr), fftInput(nullptr), fftOutput(nullptr) {
    ensureFFTInitialized();
    updateSize(width, height);
}

CornerDetector::~CornerDetector() {
    if (fftPlan) fftw_destroy_plan(fftPlan);
    if (fftInput) fftw_free(fftInput);
    if (fftOutput) fftw_free(fftOutput);
}

void CornerDetector::updateSize(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) {
        std::cerr << "Invalid size in updateSize: " << newWidth << "x" << newHeight << std::endl;
        return;
    }

    if (newWidth != width || newHeight != height) {
        width = newWidth;
        height = newHeight;
        fftOutputWidth = width / 2 + 1;
        fftOutputSize = height * fftOutputWidth;
        if (fftPlan) {
            fftw_destroy_plan(fftPlan);
            fftPlan = nullptr;
        }
        if (fftInput) {
            fftw_free(fftInput);
            fftInput = nullptr;
        }
        if (fftOutput) {
            fftw_free(fftOutput);
            fftOutput = nullptr;
        }
        int inputSize = width * height;
        int outputWidth = width / 2 + 1;
        int outputSize = height * outputWidth;

        fftInput = (double*)fftw_malloc(sizeof(double) * inputSize);
        fftOutput = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * outputSize);
        fftPlan = fftw_plan_dft_r2c_2d(height, width, fftInput, fftOutput, FFTW_ESTIMATE);

        if (!fftInput || !fftOutput) {
            std::cerr << "FFTW malloc failed in updateSize\n";
            if (fftInput) fftw_free(fftInput);
            if (fftOutput) fftw_free(fftOutput);
            fftInput = nullptr;
            fftOutput = nullptr;
            return;
        }

        if (!fftPlan) {
            std::cerr << "FFTW plan creation failed in updateSize\n";
        }
    }
}


void CornerDetector::ensureFFTInitialized() {
    if (fftPlan != nullptr) return;

    int inputSize = width * height;
    fftOutputWidth = width / 2 + 1;
    fftOutputSize = height * fftOutputWidth;

    fftInput = (double*)fftw_malloc(sizeof(double) * inputSize);
    fftOutput = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftOutputSize);

    fftPlan = fftw_plan_dft_r2c_2d(height, width, fftInput, fftOutput, FFTW_ESTIMATE);
}

std::vector<std::pair<int, int>> CornerDetector::ApplySobel(const unsigned char* grayImage, int width, int height, float threshold) {
    std::vector<std::pair<int, int>> corners;

    int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float gradX = 0.0f;
            float gradY = 0.0f;

            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    int px = x + i;
                    int py = y + j;
                    float pixel = static_cast<float>(grayImage[py * width + px]);

                    gradX += gx[j + 1][i + 1] * pixel;
                    gradY += gy[j + 1][i + 1] * pixel;
                }
            }

            float magnitude = std::sqrt(gradX * gradX + gradY * gradY);
            if (magnitude > threshold) {
                corners.emplace_back(x, y);
            }
        }
    }

    return corners;
}

void CornerDetector::prepareDataForGUI() {
    std::lock_guard<std::mutex> lock(dataMutex);

    if (lastGrayImage.empty()) {
        std::cerr << "No image data to prepare GUI." << std::endl;
        return;
    }
    guiFourierMagnitudeSpectrum = computeMagnitudeSpectrum(lastGrayImage.data());
    guiFourierPhaseCorrelation = computePhaseSpectrum(lastGrayImage.data());
    guiFourierPowerSpectralDensity = computePowerSpectralDensity(guiFourierMagnitudeSpectrum);
    guiEdgeSharpness = computeEdgeSharpness(lastGrayImage.data());
}
void CornerDetector::setGrayImage(const unsigned char* grayImage, int w, int h) {
    if (w != width || h != height) {
        updateSize(w, h);
    }

    std::lock_guard<std::mutex> lock(dataMutex);
    lastGrayImage.assign(grayImage, grayImage + (w * h));
}

void CornerDetector::captureSpectrumImage(std::vector<float> &spectrum, const char* filename) {

    float minVal = *std::min_element(spectrum.begin(), spectrum.end());
    float maxVal = *std::max_element(spectrum.begin(), spectrum.end());
    float range = (maxVal - minVal > 1e-5f) ? (maxVal - minVal) : 1.0f;

    std::vector<unsigned char> graySpectrum(spectrum.size());

    for (size_t i = 0; i < spectrum.size(); ++i) {
        float normalized = (spectrum[i] - minVal) / range;
        graySpectrum[i] = static_cast<unsigned char>(normalized * 255.0f);
    }


    int specWidth = width/2+1;
    int specHeight = height;

    ImageCapture::saveGreyImage(specWidth, specHeight, graySpectrum, filename);

}

float CornerDetector::computeEdgeSharpness(const unsigned char* grayImage) {
    int w = width;
    int h = height;

    // Sobel kernels
    int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    double sumGradMagnitude = 0.0;
    int count = 0;

    for (int y = 1; y < h - 1; y++) {
        for (int x = 1; x < w - 1; x++) {
            double gradX = 0.0;
            double gradY = 0.0;

            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    int px = x + i;
                    int py = y + j;
                    int pixelVal = grayImage[py * w + px];
                    gradX += gx[j + 1][i + 1] * pixelVal;
                    gradY += gy[j + 1][i + 1] * pixelVal;
                }
            }

            double gradMag = std::sqrt(gradX * gradX + gradY * gradY);
            sumGradMagnitude += gradMag;
            count++;
        }
    }

    return (count > 0) ? static_cast<float>(sumGradMagnitude / count) : 0.0f;
}
std::vector<float> CornerDetector::computeMagnitudeSpectrum(const unsigned char* grayImage) {
    ensureFFTInitialized();

    int inputSize = width * height;

    for (int i = 0; i < inputSize; ++i)
        fftInput[i] = static_cast<double>(grayImage[i]);

    fftw_execute(fftPlan);

    std::vector<float> magSpectrum(fftOutputSize, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < fftOutputWidth; ++x) {
            int idx = y * fftOutputWidth + x;
            double real = fftOutput[idx][0];
            double imag = fftOutput[idx][1];
            double mag = std::sqrt(real * real + imag * imag);
            magSpectrum[idx] = static_cast<float>(std::log(1.0 + mag));
        }
    }

    return magSpectrum;
}


std::vector<float> CornerDetector::computePhaseSpectrum(const unsigned char* grayImage) {
    ensureFFTInitialized();

    int inputSize = width * height;

    for (int i = 0; i < inputSize; ++i)
        fftInput[i] = static_cast<double>(grayImage[i]);

    fftw_execute(fftPlan);

    std::vector<float> phaseSpectrum(fftOutputSize, 0.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < fftOutputWidth; ++x) {
            int idx = y * fftOutputWidth + x;
            double real = fftOutput[idx][0];
            double imag = fftOutput[idx][1];
            double phase = std::atan2(imag, real);
            phaseSpectrum[idx] = static_cast<float>((phase + M_PI) / (2 * M_PI));
        }
    }

    return phaseSpectrum;
}


std::vector<float> CornerDetector::computePowerSpectralDensity(const std::vector<float>& magnitudeSpectrum) {
    std::vector<float> psd(magnitudeSpectrum.size());
    for (size_t i = 0; i < magnitudeSpectrum.size(); ++i) {
        psd[i] = magnitudeSpectrum[i] * magnitudeSpectrum[i];
    }
    return psd;
}

std::vector<float> CornerDetector::getGuiFourierMagnitudeSpectrum() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return guiFourierMagnitudeSpectrum;
}

std::vector<float> CornerDetector::getGuiFourierPhaseCorrelation() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return guiFourierPhaseCorrelation;
}

std::vector<float> CornerDetector::getGuiFourierPowerSpectralDensity() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return guiFourierPowerSpectralDensity;
}

float CornerDetector::getGuiEdgeSharpness() const {
    std::lock_guard<std::mutex> lock(dataMutex);
    return guiEdgeSharpness;
}

std::vector<float> CornerDetector::getMagnitudeSpectrumDescriptor(const unsigned char* img) {
    return computeMagnitudeSpectrum(img);
}


std::vector<float> CornerDetector::getPhaseCorrelationDescriptor(const unsigned char* img) {
    return computePhaseSpectrum(img);
}


std::vector<float> CornerDetector::getPowerSpectralDensityDescriptor(const unsigned char* img) {
    auto magSpec = computeMagnitudeSpectrum(img);
    return computePowerSpectralDensity(magSpec);
}

std::vector<unsigned char> CornerDetector::sobelVisualizer(const unsigned char *grayImage, int width, int height,
    float threshold, bool overLayOriginal) {
    auto corners = ApplySobel(grayImage,width,height,threshold);
    std::vector<unsigned char> result(
        grayImage,
        grayImage + width * height
        );
    if (overLayOriginal) {
        std::fill(result.begin(), result.end(), 0);
    }
    for (const auto& c: corners) {
        int x = c.first;
        int y = c.second;
        result[y * width + x] = 255;
    }
    return result;

}
