#define GLEW_STATIC


#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "../header/Shader.h"
#include "../header/object_data.h"
#include "../header/Display.h"
#include "../header/ImageCapture.h"
#include <direct.h>
using namespace std;
using namespace glm;


//Camera vectors
vec3 cameraPos = vec3(0.0f, 0.0f, 3.0f);
vec3 cameraFront = vec3(0.0f, 0.0f, -1.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat  lastX = 400, lastY = 300;
GLfloat yaw1 = -90.0f;
GLfloat pitch1 = 0.0f;
int newWidth, newHeight;
int oldWidth=800, oldHeight=600;
bool keys[1024];
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void movement();
void Resize(GLFWwindow* window, int width, int height);


    int main() {
        //Current working directory call
        char cwd[1024];
        _getcwd(cwd, sizeof(cwd));
        std::cout << "\nCurrent working directory: " << cwd << std::endl;

    DISPLAY display;
        display.createWindow(oldWidth,oldHeight);
        GLFWwindow* window = display.getWindow();
        Resize(window,oldWidth,oldHeight);
        newWidth = oldWidth;
        newHeight = oldHeight;

        glewExperimental = true;
        if (glewInit() != GLEW_OK) {
            cout << "Failed to initialize GLEW" << endl;
        }

        glViewport(0,0,800,600);
        glEnable(GL_DEPTH_TEST);

        glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
        glfwSetKeyCallback(window, key_callback);
        glfwSetCursorPosCallback(window, mouse_callback);

        SHADER shader ("../shader/shader.vert", "../shader/shader.frag");


        GLuint VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)nullptr);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1,2,GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);

        GLuint texture0, texture1;
        glGenTextures(1, &texture0);
        glGenTextures(1, &texture1);


        glBindTexture(GL_TEXTURE_2D, texture0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height;
        unsigned char* image = SOIL_load_image("../images/black.png", &width, &height, nullptr, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        if (!image) {
            cerr<<"Failed to load image 0"<<endl;
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);

        glBindTexture(GL_TEXTURE_2D, 0);


        glBindTexture(GL_TEXTURE_2D, texture1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        image = SOIL_load_image("../images/himohoikkahomer.png", &width, &height, nullptr, SOIL_LOAD_RGB);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        if (!image) {
            cerr << "Failed to load image 1" << endl;
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        SOIL_free_image_data(image);
        glBindTexture(GL_TEXTURE_2D, 0);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cout << "OpenGL Error: " << err << std::endl;
        }
        while (!glfwWindowShouldClose(window)) {

            GLfloat currentFrame = glfwGetTime();
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            glfwPollEvents();
            movement();
            if (oldWidth != newWidth || oldHeight != newHeight) {
                Resize(window, newWidth, newHeight);
                oldWidth = newWidth;
                oldHeight = newHeight;
            }

            glClearColor(0.3f, 0.1f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.UseShader();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture0);

            glUniform1i(glGetUniformLocation(shader.program, "texture0"), 0);
            glActiveTexture(GL_TEXTURE1);

            glBindTexture(GL_TEXTURE_2D, texture1);
            glUniform1i(glGetUniformLocation(shader.program, "texture1"), 1);


            glm::mat4 projection, view;
            view = glm::lookAt(cameraPos,cameraPos + cameraFront, cameraUp);
            projection = glm::perspective(radians(45.0f), (GLfloat)oldWidth /(GLfloat) oldHeight,0.1f,100.0f);

                GLint modelLoc = glGetUniformLocation(shader.program, "model");
                GLint viewLoc = glGetUniformLocation(shader.program, "view");
                GLint projLoc = glGetUniformLocation(shader.program, "projection");



            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glBindVertexArray(VAO);

            for (GLuint i = 0; i < 10; i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                model = glm::rotate(model, radians((float)glfwGetTime()*(10.0f * i + 10.0f)), glm::vec3(1.0f, 0.3f, 0.5f));
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));;
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }



            glBindVertexArray(0);
            glfwSwapBuffers(window);
        }
        glfwTerminate();
        return 0;
    }


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (action == GLFW_PRESS)
        keys[key] = true;
    if (action == GLFW_RELEASE)
        keys[key] = false;

}
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xPos, double yPos) {
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
        pitch1 = 89.0f;
    }
    glm::vec3 front;
    front.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch1));
    front.y = sin(glm::radians(pitch1));
    front.z = sin(glm::radians(yaw1))* cos(glm::radians(pitch1));
    cameraFront = glm::normalize(front);

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
    if (keys[GLFW_KEY_1]) {
        newWidth = 1024;
        newHeight = 768;
    }
    if (keys[GLFW_KEY_2]) {
        newWidth = 1280;
        newHeight = 720;
    }
    if (keys[GLFW_KEY_3]) {
        newWidth = 1600;
        newHeight = 1024;
    }
    if (keys[GLFW_KEY_4]) {
        newWidth = 1920;
        newHeight = 1080;
    }
    if (keys[GLFW_KEY_0]) {
        saveScreenShot(newWidth,newHeight,"screenshot.tga");
    }


}
void Resize(GLFWwindow* window, int width, int height) {
GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwSetWindowPos(window,(mode->width - width)/2,(mode->height-height)/2);
    glfwSetWindowSize(window,width,height);
    glViewport(0, 0, width, height);

}
