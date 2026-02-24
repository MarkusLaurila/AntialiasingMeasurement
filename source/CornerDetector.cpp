#include "CornerDetector.h"
#include <iostream>
#include <algorithm>
#include "ImageCapture.h"
#include <complex>
#include <cmath>
CornerDetector::CornerDetector(int width, int height)
    : width(width), height(height){
    ensureFFTInitialized();
    ensureIFFTInitialized();
    updateSize(width, height);
}

CornerDetector::~CornerDetector() {
    if (fftPlan) fftw_destroy_plan(fftPlan);
    if (ifftPlan) fftw_destroy_plan(ifftPlan);
    if (fftInput) fftw_free(fftInput);
    if (fftOutput) fftw_free(fftOutput);
    if (ifftOutput) fftw_free(ifftOutput);
}

void CornerDetector::updateSize(int newWidth, int newHeight) {
    if (newWidth <= 0 || newHeight <= 0) return;

    if (newWidth != width || newHeight != height) {
        width = newWidth;
        height = newHeight;
        fftOutputWidth = width / 2 + 1;
        fftOutputSize = height * fftOutputWidth;
        int inputSize = width * height;

        if (fftPlan) { fftw_destroy_plan(fftPlan); fftPlan = nullptr; }
        if (ifftPlan) { fftw_destroy_plan(ifftPlan); ifftPlan = nullptr; }

        if (fftInput) { fftw_free(fftInput); fftInput = nullptr; }
        if (fftOutput) { fftw_free(fftOutput); fftOutput = nullptr; }
        if (ifftOutput) { fftw_free(ifftOutput); ifftOutput = nullptr; }

        fftInput = (double*)fftw_malloc(sizeof(double) * inputSize);
        fftOutput = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * fftOutputSize);
        ifftOutput = (double*)fftw_malloc(sizeof(double) * inputSize);

        if (!fftInput || !fftOutput || !ifftOutput) {
            std::cerr << "FFTW malloc failed in updateSize\n";
            return;
        }
        fftPlan = fftw_plan_dft_r2c_2d(height, width, fftInput, fftOutput, FFTW_ESTIMATE);
        ifftPlan = fftw_plan_dft_c2r_2d(height, width, fftOutput, ifftOutput, FFTW_ESTIMATE);

        if (!fftPlan || !ifftPlan)
            std::cerr << "FFTW plan creation failed in updateSize\n";
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

void CornerDetector::ensureIFFTInitialized() {
    if (ifftPlan != nullptr) return;
    int inputSize = width * height;
    ifftOutput = (double*)fftw_malloc(sizeof(double) * inputSize);
    ifftPlan = fftw_plan_dft_c2r_2d(
        height, width,
        fftOutput,
        ifftOutput,
        FFTW_ESTIMATE
    );
    if (!ifftPlan || !ifftOutput) {
        std::cerr << "FFTW inverse plan or buffer creation failed\n";
        if (ifftOutput) fftw_free(ifftOutput);
        ifftOutput = nullptr;
    }
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
void CornerDetector::prepareDataForGUI(AAType aaType) {
    std::lock_guard<std::mutex> lock(dataMutex);

    guiFourierMagnitudeSpectrum = computeMagnitudeSpectrum();
    guiFourierPhaseCorrelation = computePhaseCorrelation(aaType);
    guiFourierPowerSpectralDensity = computePowerSpectralDensity();
    guiEdgeSharpness = computeEdgeSharpness((aaType == AAType::SSAA) ? referenceSSAA.data() : referenceNoAA.data());
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


std::vector<float> CornerDetector::computeMagnitudeSpectrum() {
    ensureFFTInitialized();

    if (lastGrayImage.empty()) return {};

    int inputSize = width * height;
    for (int i = 0; i < inputSize; ++i)
        fftInput[i] = static_cast<double>(lastGrayImage[i]);
    fftw_execute(fftPlan);

    std::vector<float> magSpectrum(fftOutputSize);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < fftOutputWidth; ++x) {
            int idx = y * fftOutputWidth + x;
            double real = fftOutput[idx][0];
            double imag = fftOutput[idx][1];
            magSpectrum[idx] = static_cast<float>(std::log(1.0 + std::sqrt(real*real + imag*imag)));
        }
    }
    return magSpectrum;
}

std::vector<float> CornerDetector::computePhaseCorrelation(AAType aaType) {
    ensureFFTInitialized();
    ensureIFFTInitialized();

    const std::vector<unsigned char>& refImage = (aaType == AAType::SSAA) ? referenceSSAA : referenceNoAA;
    if (refImage.empty() || lastGrayImage.empty()) return {};

    int inputSize = width * height;
    std::vector<double> currentInput(inputSize);
    std::vector<double> referenceInput(inputSize);
    std::vector<std::complex<double>> currentFFT(fftOutputSize);
    std::vector<std::complex<double>> referenceFFT(fftOutputSize);

    for (int i = 0; i < inputSize; ++i) currentInput[i] = static_cast<double>(lastGrayImage[i]);
    fftw_execute_dft_r2c(fftPlan, currentInput.data(), reinterpret_cast<fftw_complex*>(currentFFT.data()));

    for (int i = 0; i < inputSize; ++i) referenceInput[i] = static_cast<double>(refImage[i]);
    fftw_execute_dft_r2c(fftPlan, referenceInput.data(), reinterpret_cast<fftw_complex*>(referenceFFT.data()));

    for (int i = 0; i < fftOutputSize; ++i) {
        std::complex<double> R = currentFFT[i] * std::conj(referenceFFT[i]);
        double mag = std::abs(R);
        if (mag > 1e-12) {
            fftOutput[i][0] = R.real() / mag;
            fftOutput[i][1] = R.imag() / mag;
        } else {
            fftOutput[i][0] = 0.0;
            fftOutput[i][1] = 0.0;
        }
    }



    fftw_execute(ifftPlan);
    std::vector<float> correlation(inputSize);
    for (int i = 0; i < inputSize; ++i) correlation[i] = static_cast<float>(ifftOutput[i] / inputSize);

    return correlation;
}

std::vector<float> CornerDetector::computePowerSpectralDensity() {
    auto magSpec = computeMagnitudeSpectrum();
    std::vector<float> psd(magSpec.size());
    for (size_t i = 0; i < magSpec.size(); ++i)
        psd[i] = magSpec[i] * magSpec[i];
    return psd;
}
void CornerDetector::setReferenceImageNoAA(const unsigned char* referenceImage, int w, int h) {
    referenceNoAA.assign(referenceImage, referenceImage + w * h);
    width = w;
    height = h;
}

void CornerDetector::setReferenceImageSSAA(const unsigned char* referenceImage, int w, int h) {
    referenceSSAA.assign(referenceImage, referenceImage + w * h);
    width = w;
    height = h;
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

// std::vector<float> CornerDetector::getMagnitudeSpectrumDescriptor(const unsigned char* img) {
//     return computeMagnitudeSpectrum(img);
// }
//
//
// std::vector<float> CornerDetector::getPhaseCorrelationDescriptor(const unsigned char* img) {
//     return computePhaseSpectrum(img);
// }
//
//
// std::vector<float> CornerDetector::getPowerSpectralDensityDescriptor(const unsigned char* img) {
//     auto magSpec = computeMagnitudeSpectrum(img);
//     return computePowerSpectralDensity(magSpec);
// }

std::vector<unsigned char> CornerDetector::sobelVisualizerRGB(
    const unsigned char* grayImage,
    int width,
    int height,
    float threshold,
    bool overlayCorners)
{
    std::vector<unsigned char> output(width * height * 3);

    const int gx[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };
    const int gy[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {

            int sx = 0, sy = 0;

            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {

                    unsigned char p = grayImage[(y + j) * width + (x + i)];

                    sx += p * gx[j + 1][i + 1];
                    sy += p * gy[j + 1][i + 1];
                }
            }

            float mag = std::sqrt(float(sx * sx + sy * sy));
            mag = std::min(255.0f, mag);

            int idx = (y * width + x) * 3;

            output[idx + 0] = (unsigned char)std::min(255, std::abs(sx));
            output[idx + 1] = (unsigned char)std::min(255, std::abs(sy));
            output[idx + 2] = (unsigned char)mag;
        }
    }
    if (overlayCorners) {
        auto corners = ApplySobel(grayImage, width, height, threshold);
        for (auto& c : corners) {
            int idx = (c.second * width + c.first) * 3;
            output[idx + 0] = 255;
            output[idx + 1] = 0;
            output[idx + 2] = 0;
        }
    }

    return output;
}
std::vector<unsigned char> CornerDetector::sobelVisualizerGrey(
    const unsigned char* grayImage, int width, int height, float threshold, bool overlayOriginal)
{
    std::vector<unsigned char> result(grayImage, grayImage + width * height);

    int gx[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    int gy[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};

    for (int y = 1; y < height-1; ++y) {
        for (int x = 1; x < width-1; ++x) {
            int sx = 0, sy = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixel = grayImage[(y + ky) * width + (x + kx)];
                    sx += gx[ky+1][kx+1] * pixel;
                    sy += gy[ky+1][kx+1] * pixel;
                }
            }

            float mag = std::sqrt(float(sx*sx + sy*sy));
            unsigned char value = static_cast<unsigned char>(std::min(255.0f, mag));

            if (overlayOriginal) {
                result[y * width + x] = std::max(grayImage[y * width + x], value);
            } else {
                result[y * width + x] = value;
            }
        }
    }

    return result;
}





