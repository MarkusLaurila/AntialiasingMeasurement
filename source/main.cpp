
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <thread>
#include <mutex>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "TextureManager.h"
#include "Shader.h"
#include "Display.h"
#include "OverLay.h"
#include "ImageCapture.h"
#include "CornerDetector.h"
#include "SkyBox.h"
#include "ModelImporter.h"
#include <direct.h>
using namespace std;
using namespace glm;

float quadVertices[] = {

    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,

    -1.0f,  1.0f,  0.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

//Camera vectors
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
GLuint quadVAO = 0, quadVBO = 0;
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat  lastX = 400, lastY = 300;
GLfloat yaw1 = -90.0f;
GLfloat pitch1 = 0.0f;
int newWidth, newHeight;
int oldWidth=800, oldHeight=600;
bool keys[1024];
int currentAA = 0;
int frameIndex = 0;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void movement();
void Resize(GLFWwindow* window, int width, int height);
float Halton(int index, int base);
void setupScreenQuad();
void renderScreenQuad();
bool detectCorners = false;
bool fourierTransform = false;
bool cursorEnabled = false;
std::mutex overlayMutex;
vector<string> faces

{ //Low-res for testing  TODO:Swap to highres-skybox for images
    "../resources/Lowres-skybox/px.png",
    "../resources/Lowres-skybox/nx.png",
    "../resources/Lowres-skybox/py.png",
    "../resources/Lowres-skybox/ny.png",
    "../resources/Lowres-skybox/pz.png",
    "../resources/Lowres-skybox/nz.png"
};


    int main() {

        char cwd[1024];
        _getcwd(cwd, sizeof(cwd));
        std::cout << "\nCurrent working directory: " << cwd << std::endl;

    DISPLAY display;
    OverLay overlay;
        display.createWindow(oldWidth,oldHeight);
        GLFWwindow* window = display.getWindow();
        Resize(window,oldWidth,oldHeight);
        overlay.initGUI(window);

        newWidth = oldWidth;
        newHeight = oldHeight;

        CornerDetector cornerDetector(newWidth,newHeight);
        glfwSwapInterval(0);
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            cout << "Failed to initialize GLEW" << endl;
        }
        glViewport(0,0,800,600);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        Skybox skybox(faces);
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        ModelImporter model_importer;
        //OpenGL init check
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << err << std::endl;
        }
        if (model_importer.loadModel("../resources/models/TowerIsland.fbx")) {

            std::cout << "Model loaded successfully."  << std::endl;

        }
        else {
            std::cout << "Failed to load model" << std::endl;
            glfwTerminate();
            return 0;
        }
        const auto& meshes = model_importer.getMeshes();


        SHADER sceneShader ("../shader/shader.vert", "../shader/shader.frag");
        SHADER aaShader ("../shader/screenQuad.vert", "../shader/shaderAntiAliasing.frag");

        GLuint fbo, fboTexture, rbo, historyTexture;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        glGenTextures(1, &historyTexture);
        glBindTexture(GL_TEXTURE_2D, historyTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, oldWidth, oldHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cerr << "Framebuffer not complete!" << endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLuint taaFBO, taaTexture;
        glGenFramebuffers(1, &taaFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, taaFBO);

        glGenTextures(1, &taaTexture);
        glBindTexture(GL_TEXTURE_2D, taaTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, taaTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cerr << "TAA Framebuffer not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        setupScreenQuad();


        while (!glfwWindowShouldClose(window)) {
            GLfloat currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;
            glfwPollEvents();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            if (!ImGui::GetIO().WantCaptureKeyboard){movement();}



            if (oldWidth != newWidth || oldHeight != newHeight) {
                oldWidth = newWidth;
                oldHeight = newHeight;
                glBindTexture(GL_TEXTURE_2D, fboTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);


                glBindRenderbuffer(GL_RENDERBUFFER, rbo);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, oldWidth, oldHeight);

                if(historyTexture != 0 && currentAA == 4) {
                    glBindTexture(GL_TEXTURE_2D, historyTexture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                }
                glBindTexture(GL_TEXTURE_2D, 0);
                glBindRenderbuffer(GL_RENDERBUFFER, 0);
                glBindTexture(GL_TEXTURE_2D, taaTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, oldWidth, oldHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                Resize(window, oldWidth, oldHeight);
                cornerDetector.updateSize(oldWidth, oldHeight);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glEnable(GL_DEPTH_TEST);
            glClearColor(0.3f, 0.1f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            sceneShader.UseShader();
            vec3 lightDir = normalize(vec3(-0.5f, -1.0f, -0.3f));
            vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

            glUniform3fv(glGetUniformLocation(sceneShader.program, "lightDir"), 1, value_ptr(lightDir));
            glUniform3fv(glGetUniformLocation(sceneShader.program, "lightColor"), 1, value_ptr(lightColor));
            //Scale for model
            mat4 model = scale(mat4(0.4f), vec3(0.1f));
            model = rotate(model,radians(90.0f), vec3(-1.0f, 0.0f, 0.0f));


            mat4 view = lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            float jitterX = Halton((frameIndex % 8) + 1, 2) - 0.5f;
            float jitterY = Halton((frameIndex % 8) + 1, 3) - 0.5f;

            vec2 jitter(jitterX , jitterY );
            vec2 jitterUV = jitter / vec2(oldWidth, oldHeight);

            frameIndex++;
            mat4 projection = perspective(radians(45.0f), (float)oldWidth / (float)oldHeight, 0.1f, 100.0f);
            if (currentAA == 4) {
                projection[2][0] += jitterUV.x* 2.0f;
                projection[2][1] += jitterUV.y* 2.0f;
            }
            glUniformMatrix4fv(glGetUniformLocation(sceneShader.program, "view"), 1, GL_FALSE, value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(sceneShader.program, "projection"), 1, GL_FALSE, value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(sceneShader.program, "model"), 1, GL_FALSE, value_ptr(model));

            glBindVertexArray(0);

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


            for (const auto& mesh : meshes) {
                if (mesh.materialTextures.baseColorID != 0) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, mesh.materialTextures.baseColorID);
                    glUniform1i(glGetUniformLocation(sceneShader.program, "material.diffuse"), 0);
                }
                if (mesh.materialTextures.normalID != 0) {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, mesh.materialTextures.normalID);
                    glUniform1i(glGetUniformLocation(sceneShader.program, "material.normal"), 1);
                }
                if (mesh.materialTextures.roughnessID != 0) {
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, mesh.materialTextures.roughnessID);
                    glUniform1i(glGetUniformLocation(sceneShader.program, "material.roughness"), 2);
                }
                if (mesh.materialTextures.aoID != 0) {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, mesh.materialTextures.aoID);
                    glUniform1i(glGetUniformLocation(sceneShader.program, "material.ao"), 3);
                }
                mesh.Draw();
            }


             glDepthFunc(GL_LEQUAL);
             glDepthMask(GL_FALSE);
             skybox.Draw(view, projection);
             glDepthMask(GL_TRUE);
             glDepthFunc(GL_LESS);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDisable(GL_DEPTH_TEST);

            switch (currentAA) {
                case 0: {
                    aaShader.UseShader();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    glUniform2f(glGetUniformLocation(aaShader.program, "screenSize"), (float)oldWidth, (float)oldHeight);
                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 0);
                    renderScreenQuad();


                    break;
                }
                case 1: {
                    aaShader.UseShader();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    glUniform2f(glGetUniformLocation(aaShader.program, "screenSize"), (float)oldWidth, (float)oldHeight);
                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 1);
                    renderScreenQuad();
                    break;
                }
                case 2: {
                    aaShader.UseShader();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    glUniform2f(glGetUniformLocation(aaShader.program, "screenSize"), (float)oldWidth, (float)oldHeight);
                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 2);
                     renderScreenQuad();
                    break;
                }
                case 3: {
                    aaShader.UseShader();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    glUniform2f(glGetUniformLocation(aaShader.program, "screenSize"), (float)oldWidth, (float)oldHeight);
                    glUniform1i(glGetUniformLocation(aaShader.program, "sampleCount"), 4);
                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 3);

                    renderScreenQuad();
                    break;
                }
                case 4: {

                    glBindFramebuffer(GL_FRAMEBUFFER, taaFBO);
                    glDisable(GL_DEPTH_TEST);
                    glClear(GL_COLOR_BUFFER_BIT);

                    aaShader.UseShader();
                    glUniform2f(glGetUniformLocation(aaShader.program, "jitter"), jitterUV.x, jitterUV.y);
                    glUniform1f(glGetUniformLocation(aaShader.program, "blendFactor"), 0.1f);

                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);

                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, historyTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "historyTexture"), 1);

                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 4);
                    renderScreenQuad();


                    glBindFramebuffer(GL_READ_FRAMEBUFFER, taaFBO);
                    glBindTexture(GL_TEXTURE_2D, historyTexture);
                    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, oldWidth, oldHeight);
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
                    glBindTexture(GL_TEXTURE_2D, 0);


                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    aaShader.UseShader();
                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, taaTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    renderScreenQuad();


                break;

                }
                default: {
                    aaShader.UseShader();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, fboTexture);
                    glUniform1i(glGetUniformLocation(aaShader.program, "screenTexture"), 0);
                    glUniform2f(glGetUniformLocation(aaShader.program, "screenSize"), (float)oldWidth, (float)oldHeight);

                    glUniform1i(glGetUniformLocation(aaShader.program, "currentAA"), 0);
                renderScreenQuad();

                }

                    break;
            }



            if (detectCorners) {

                std::vector<unsigned char> pixels(newWidth * newHeight * 3);
                glReadPixels(0, 0, newWidth, newHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

                std::vector<unsigned char> gray(newWidth * newHeight);
                for (int i = 0; i < newWidth * newHeight; ++i) {
                    int r = pixels[3 * i];
                    int g = pixels[3 * i + 1];
                    int b = pixels[3 * i + 2];
                    gray[i] = static_cast<unsigned char>(0.299f * r + 0.587f * g + 0.114f * b);
                }
                auto corners = CornerDetector::ApplySobel(gray.data(), newWidth, newHeight, overlay.threshold);
                //Save sobelVisualizer
              //  auto sobelVisualizer = CornerDetector::sobelVisualizer(gray.data(), newWidth, newHeight, overlay.threshold);
              //  ImageCapture::saveGreyImage(newWidth,newHeight,sobelVisualizer,"SobelVisual.tga");
                overlay.detectedCornersCount = corners.size();

            }
            if (fourierTransform) {
                std::vector<unsigned char> pixels(newWidth * newHeight * 3);
                glReadPixels(0, 0, newWidth, newHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

                std::vector<unsigned char> gray(newWidth * newHeight);
                for (int i = 0; i < newWidth * newHeight; ++i) {
                    int r = pixels[3 * i];
                    int g = pixels[3 * i + 1];
                    int b = pixels[3 * i + 2];
                    gray[i] = static_cast<unsigned char>(0.299f * r + 0.587f * g + 0.114f * b);
                }
                ImageCapture::saveScreenShot(newWidth, newHeight, "NormalScreenshot.tga");

                std::thread([gray = std::move(gray), &overlay, &cornerDetector]() {
                    cornerDetector.setGrayImage(gray.data(), newWidth, newHeight);
                    cornerDetector.prepareDataForGUI();
                    auto magSpec = cornerDetector.getGuiFourierMagnitudeSpectrum();
                    auto phaseCorr = cornerDetector.getGuiFourierPhaseCorrelation();
                    auto psd = cornerDetector.getGuiFourierPowerSpectralDensity();
                    auto edgeSharp = cornerDetector.getGuiEdgeSharpness();

                    //Images for paper TODO:REMOVE
                    // cornerDetector.captureSpectrumImage(magSpec,"fourier_Magnitude.tga");
                    // cornerDetector.captureSpectrumImage(phaseCorr,"fourier_PhaseCorrelation.tga");
                    // cornerDetector.captureSpectrumImage(psd,"fourier_PowerSpectralDensity.tga");


                    std::lock_guard<std::mutex> lock(overlayMutex);
                    overlay.fourierMagnitudeSpectrum = std::move(magSpec);
                    overlay.fourierPhaseCorrelation = std::move(phaseCorr);
                    overlay.fourierPowerSpectralDensity = std::move(psd);
                    overlay.edgeSharpness = edgeSharp;

                    std::cerr << "Fourier transform calculations done." << std::endl;
                }).detach();
            } {
                overlay.graphSize = ImVec2(480, 240); // Change size of graph
                std::lock_guard<std::mutex> lock(overlayMutex);
               overlay.renderGUI(cursorEnabled,currentAA, fourierTransform);
                fourierTransform = false;
                detectCorners = false;
            }
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }

         glDeleteVertexArrays(1, &quadVAO);
         glDeleteBuffers(1, &quadVBO);
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &fboTexture);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &taaFBO);
        glDeleteTextures(1, &taaTexture);
        TextureManager::Clear();
        overlay.shutDownGUI();
        glfwTerminate();
        return 1;
    }









//Translate glfw to imgui
ImGuiKey ImGui_ImplGlfw_KeyToImGuiKey(int key)
    {
        switch (key)
        {
            case GLFW_KEY_TAB: return ImGuiKey_Tab;
            case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
            case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
            case GLFW_KEY_UP: return ImGuiKey_UpArrow;
            case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
            case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
            case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
            case GLFW_KEY_HOME: return ImGuiKey_Home;
            case GLFW_KEY_END: return ImGuiKey_End;
            case GLFW_KEY_INSERT: return ImGuiKey_Insert;
            case GLFW_KEY_DELETE: return ImGuiKey_Delete;
            case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
            case GLFW_KEY_SPACE: return ImGuiKey_Space;
            case GLFW_KEY_ENTER: return ImGuiKey_Enter;
            case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
            case GLFW_KEY_A: return ImGuiKey_A;
            case GLFW_KEY_C: return ImGuiKey_C;
            case GLFW_KEY_V: return ImGuiKey_V;
            case GLFW_KEY_X: return ImGuiKey_X;
            case GLFW_KEY_Y: return ImGuiKey_Y;
            case GLFW_KEY_Z: return ImGuiKey_Z;
            case GLFW_KEY_F: return ImGuiKey_F;
            default: return ImGuiKey_None;
        }
    }
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiMod_Ctrl,  (mode & GLFW_MOD_CONTROL) != 0);
        io.AddKeyEvent(ImGuiMod_Shift, (mode & GLFW_MOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiMod_Alt,   (mode & GLFW_MOD_ALT) != 0);
        io.AddKeyEvent(ImGuiMod_Super, (mode & GLFW_MOD_SUPER) != 0);


        if (key >= GLFW_KEY_SPACE && key <= GLFW_KEY_LAST) {
            ImGuiKey imguiKey = ImGui_ImplGlfw_KeyToImGuiKey(key);
            if (imguiKey != ImGuiKey_None) {
                io.AddKeyEvent(imguiKey, action != GLFW_RELEASE);
            }
        }

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);

        if (action == GLFW_PRESS) {
            keys[key] = true;
            switch (key) {
                case GLFW_KEY_1:
                    newWidth = 1024; newHeight = 768; break;
                case GLFW_KEY_2:
                    newWidth = 1280; newHeight = 720; break;
                case GLFW_KEY_3:
                    newWidth = 1600; newHeight = 1024; break;
                case GLFW_KEY_4:
                    newWidth = 1920; newHeight = 1080; break;
                case GLFW_KEY_5:
                    currentAA = 0; std::cout << "No AA enabled" << std::endl; break;
                case GLFW_KEY_6:
                    currentAA = 1; std::cout << "FXAA enabled" << std::endl; break;
                case GLFW_KEY_7:
                    currentAA = 2; std::cout << "SSAA enabled" << std::endl; break;
                case GLFW_KEY_8:
                    currentAA = 3; std::cout << "MSAA enabled" << std::endl; break;
                case GLFW_KEY_9:
                    currentAA = 4; std::cout << "TAA enabled" << std::endl; break;
                case GLFW_KEY_0:
                    break;
                case GLFW_KEY_C:
                    detectCorners = !detectCorners;
                    break;
                case GLFW_KEY_F:

                    fourierTransform = !fourierTransform;

                    break;
                case GLFW_KEY_TAB:
                    cursorEnabled = !cursorEnabled;
                    glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                    break;

                    default:
                    break;
            }
        }

        if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
    }

bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {

    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2((float)xPos, (float)yPos);
    if (cursorEnabled || io.WantCaptureMouse) return;

    if (cursorEnabled) return;
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    lastX = xPos;
    lastY = yPos;

    GLfloat sensitivity = 0.05f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw1 += xOffset;
    pitch1 += yOffset;

    if (pitch1 > 89.0f) {
        pitch1 = 89.0f;
    }
    if (pitch1 < -89.0f) {
        pitch1 = -89.0f;
    }
    glm::vec3 front;
    front.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch1));
    front.y = sin(glm::radians(pitch1));
    front.z = sin(glm::radians(yaw1))* cos(glm::radians(pitch1));
    cameraFront = glm::normalize(front);

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGui::GetIO().AddMouseWheelEvent((float)xoffset, (float)yoffset);
}
void movement() {
    GLfloat cameraSpeed = 5.0f * deltaTime;

    if (keys[GLFW_KEY_W]) {
        cameraPos += cameraSpeed * cameraFront;

    }
    if (keys[GLFW_KEY_S]) {
        cameraPos -= cameraSpeed * cameraFront;

    }
    if (keys[GLFW_KEY_A]) {
        cameraPos -= glm::normalize(glm::cross(cameraFront,cameraUp)) * cameraSpeed;
    }
    if (keys[GLFW_KEY_D]) {
        cameraPos += glm::normalize(glm::cross(cameraFront,cameraUp)) * cameraSpeed;
    }


}
void Resize(GLFWwindow* window, int width, int height) {
GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window,(mode->width - width)/2,(mode->height-height)/2);
    glfwSetWindowSize(window,width,height);

    glViewport(0, 0, width, height);


}
float Halton(int index, int base) {
    float result = 0.0f;
    float f = 1.0f / (float)base;
    int i = index;
    while(i > 0) {
        result += f * (i % base);
        i = i / base;
        f = f / base;
    }
    return result;
}
void setupScreenQuad() {
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}
void renderScreenQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
