#pragma once
#include <vector>
#include <fftw3.h>
#include <mutex>

#define M_PI 3.14159265358979323846
struct Point2D {
    int x, y;
};
enum class AAType {
    NoAA,
    SSAA
};
class CornerDetector {
public:
    CornerDetector(int width, int height);
    ~CornerDetector();
   void updateSize(int newWidth, int newHeight);

   static std::vector<std::pair<int, int>> ApplySobel(const unsigned char* grayImage, int width, int height, float threshold);

    void prepareDataForGUI(AAType aaType);
    std::vector<float> getGuiFourierMagnitudeSpectrum() const;
    std::vector<float> getGuiFourierPhaseCorrelation() const;
    std::vector<float> getGuiFourierPowerSpectralDensity() const;
    float getGuiEdgeSharpness() const;
    float computeEdgeSharpness(const unsigned char* grayImage);
    // std::vector<float> getMagnitudeSpectrumDescriptor(const unsigned char* img);
    // std::vector<float> getPhaseCorrelationDescriptor(const unsigned char* img);
    // std::vector<float> getPowerSpectralDensityDescriptor(const unsigned char* img);
    static std::vector<unsigned char> sobelVisualizerRGB(
        const unsigned char* grayImage,
        int width,
        int height,
        float threshold,
        bool overLayOriginal = false
        );
   static std::vector<unsigned char> sobelVisualizerGrey(
    const unsigned char* grayImage,
    int width,
    int height,
    float threshold,
    bool overlayOriginal
    );
    void setGrayImage(const unsigned char* grayImage, int w, int h);
    void setReferenceImageNoAA(const unsigned char* referenceImage, int w, int h);
    void setReferenceImageSSAA(const unsigned char* referenceImage, int w, int h);
    void captureSpectrumImage(std::vector<float>& spectrum, const char* filename);
private:
    int width, height;
    double* fftInput = nullptr;
    double* ifftOutput = nullptr;
    fftw_complex* fftOutput = nullptr;
    fftw_plan fftPlan = nullptr;
    fftw_plan ifftPlan = nullptr;
    int fftOutputWidth = 0;
    int fftOutputSize = 0;


    void ensureFFTInitialized();
    void ensureIFFTInitialized();
    std::vector<float> computeMagnitudeSpectrum();
    std::vector<float> computePhaseCorrelation(AAType aaType);
    std::vector<float> computePowerSpectralDensity();
    std::vector<unsigned char> referenceNoAA;
    std::vector<unsigned char> referenceSSAA;
    std::vector<unsigned char> lastGrayImage;
    std::vector<float> guiFourierMagnitudeSpectrum;
    std::vector<float> guiFourierPhaseCorrelation;
    std::vector<float> guiFourierPowerSpectralDensity;
    float guiEdgeSharpness = 0.0f;
    mutable std::mutex dataMutex;
};

