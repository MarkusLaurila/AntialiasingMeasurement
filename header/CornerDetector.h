#pragma once
#include <vector>
#include <cmath>
#include <glm.hpp>
#include <fftw3.h>
#include <mutex>
struct Point2D {
    int x, y;
};

class CornerDetector {
public:
    CornerDetector(int width, int height);
    ~CornerDetector();
   void updateSize(int newWidth, int newHeight);

   static std::vector<std::pair<int, int>> ApplySobel(const unsigned char* grayImage, int width, int height, float threshold);

    void prepareDataForGUI();
    std::vector<float> getGuiFourierMagnitudeSpectrum() const;
    std::vector<float> getGuiFourierPhaseCorrelation() const;
    std::vector<float> getGuiFourierPowerSpectralDensity() const;
    float getGuiEdgeSharpness() const;
    float computeEdgeSharpness(const unsigned char* grayImage);
    std::vector<float> getMagnitudeSpectrumDescriptor(const unsigned char* img);
    std::vector<float> getPhaseCorrelationDescriptor(const unsigned char* img);
    std::vector<float> getPowerSpectralDensityDescriptor(const unsigned char* img);
    static std::vector<unsigned char> sobelVisualizer(
        const unsigned char* grayImage,
        int width,
        int height,
        float threshold,
        bool overLayOriginal = true
        );
    void setGrayImage(const unsigned char* grayImage, int w, int h);
    void captureSpectrumImage(std::vector<float>& spectrum, const char* filename);
private:
    int width, height;
    double* fftInput = nullptr;
    fftw_complex* fftOutput = nullptr;
    fftw_plan fftPlan = nullptr;
    int fftOutputWidth = 0;
    int fftOutputSize = 0;

    void ensureFFTInitialized();
    std::vector<float> computeMagnitudeSpectrum(const unsigned char* grayImage);
    std::vector<float> computePhaseSpectrum(const unsigned char* grayImage);
    std::vector<float> computePowerSpectralDensity(const std::vector<float>& magnitudeSpectrum);
    std::vector<float> guiFourierMagnitudeSpectrum;
    std::vector<float> guiFourierPhaseCorrelation;
    std::vector<float> guiFourierPowerSpectralDensity;
    float guiEdgeSharpness = 0.0f;
    std::vector<unsigned char> lastGrayImage;
    mutable std::mutex dataMutex;
};

