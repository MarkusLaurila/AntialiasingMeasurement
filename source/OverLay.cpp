
#include "OverLay.h"
#include <imgui.h>
#include <iostream>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>



OverLay::OverLay() = default;
OverLay::~OverLay() = default;
//Store downsampled and compare so wont run every frame
static std::vector<float> downsampledMag, downsampledPhase, downsampledSpec;
static size_t lastMagSize = 0, lastPhaseSize = 0, lastSpecSize = 0;
static float averageMag, averagePhase, averageSpec;



void OverLay::initGUI(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");
}
auto downsample_minmax = [](const std::vector<float>& src, int maxPoints = 512) -> std::vector<float> {
    if ((int)src.size() <= maxPoints) return src;

    std::vector<float> result;
    result.reserve(maxPoints * 2);

    float step = static_cast<float>(src.size()) / maxPoints;
    for (int i = 0; i < maxPoints; ++i) {
        int start = static_cast<int>(i * step);
        int end = static_cast<int>((i + 1) * step);
        if (end > (int)src.size()) end = (int)src.size();

        float minVal = FLT_MAX;
        float maxVal = -FLT_MAX;
        for (int j = start; j < end; ++j) {
            float val = src[j];
            if (val < minVal) minVal = val;
            if (val > maxVal) maxVal = val;
        }

        result.push_back(minVal);
        result.push_back(maxVal);
    }

    return result;
};
auto average = [](const std::vector<float>& v) -> float {
    if (v.empty()) return 0.0f;
    float sum = 0.0f;
    for (float x : v) sum += x;
    return sum / v.size();
};
void OverLay::renderGUI(bool cursorEnable,int currentAA, bool fourierTransformRequested) {

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("Overlay", NULL,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    switch (currentAA) {
        case 0:ImGui::Text("Antialiasing: None"); break;
        case 1:ImGui::Text("Antialiasing: FXAA"); break;
        case 2:ImGui::Text("Antialiasing: SSAA"); break;
        case 3:ImGui::Text("Antialiasing: MSAA"); break;
        case 4:ImGui::Text("Antialiasing: TAA"); break;
        default:ImGui::Text("Antialiasing: None"); break;
    }



    ImGui::SliderFloat("Corner Threshold", &threshold, 0.0f, 1000.0f);
    ImGui::Text("Corners detected: %d", detectedCornersCount);
        if (!fourierMagnitudeSpectrum.empty()) {

            if (fourierTransformRequested || lastMagSize == 0) {

                lastMagSize = fourierMagnitudeSpectrum.size();
                downsampledMag = downsample_minmax(fourierMagnitudeSpectrum, 512);
                averageMag = average(fourierMagnitudeSpectrum);

                lastPhaseSize = fourierPhaseCorrelation.size();
                downsampledPhase = downsample_minmax(fourierPhaseCorrelation, 512);
                averagePhase = average(fourierPhaseCorrelation);

                lastSpecSize = fourierPowerSpectralDensity.size();
                downsampledSpec = downsample_minmax(fourierPowerSpectralDensity, 512);
                averageSpec = average(fourierPowerSpectralDensity);
            }
            ImGui::PlotLines("##MagPlot", downsampledMag.data(), (int)downsampledMag.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(graphSize));
            ImGui::SeparatorText("Fourier Magnitude Spectrum");
            ImGui::Text("Average: %.3f", averageMag);
            for (int i = 0; i < 5 && i < (int)downsampledMag.size(); ++i)
                ImGui::Text("[%d] = %.3f", i, downsampledMag[i]);
            ImGui::Text("Edge Sharpness: %.3f", edgeSharpness); // Edge text
            ImGui::SeparatorText("Phase Correlation");
            ImGui::PlotLines("##PhasePlot", downsampledPhase.data(), (int)downsampledPhase.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(graphSize));
            ImGui::Text("Average: %.3f", averagePhase);
            for (int i = 0; i < 5 && i < (int)downsampledPhase.size(); ++i)
                ImGui::Text("[%d] = %.3f", i, downsampledPhase[i]);
            ImGui::SeparatorText("Power Spectral Density");
            ImGui::PlotLines("##PSDPlot", downsampledSpec.data(), (int)downsampledSpec.size(), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(graphSize));
            ImGui::Text("Average: %.3f", averageSpec);
            for (int i = 0; i < 5 && i < (int)downsampledSpec.size(); ++i)
                ImGui::Text("[%d] = %.3f", i, downsampledSpec[i]);
        }


    ImGui::End();
}

void OverLay::shutDownGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


