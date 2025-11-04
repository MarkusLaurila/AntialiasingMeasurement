#pragma once
#include <vector>
#include <GLFW/glfw3.h>

#include "imgui.h"

class OverLay {
    public:
    OverLay();
    ~OverLay();
    void initGUI(GLFWwindow* window);
    void renderGUI(bool cursorEnable,int currentAA, bool fourierTransformRequested);
    void shutDownGUI();
    int detectedCornersCount = 0;
    std::vector<float> fourierMagnitudeSpectrum;
    std::vector<float>  fourierPhaseCorrelation;
    std::vector<float> fourierPowerSpectralDensity;
    float edgeSharpness = 0;
    float threshold = 200.0f;
    ImVec2 graphSize = ImVec2(0,80);
};



