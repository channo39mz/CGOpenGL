#undef GLFW_DLL
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <cmath>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"
#include "Libs/stb_image.h"

using namespace std;

const GLint WIDTH = 800, HEIGHT = 600;

Window mainWindow;
std::vector<Mesh *> meshList;
std::vector<Shader> shaderList;

float yaw = -90.0f, pitch = 0.0f;

// Vertex Shader
static const char *vShader = "Shaders/shader.vert";



// Fragment Shader
static const char *fShader = "Shaders/shader.frag";

void CreateTriangle()
{
    GLfloat vertices[] =
        {
            // pos                       //aTexCoord
            -1.0f, -1.0f,                    0.0f, 0.0f, 0.0f,
            0.0f, -1.0f,                    1.0f, 0.5f, 0.0f,
            1.0f, -1.0f,                     0.0f, 1.0f, 0.0f,
            0.0f, 1.0f,                     0.0f, 0.5f, 1.0f};

    unsigned int indices[] =
        {
            0, 3, 1,
            1, 3, 2,
            2, 3, 0,
            0, 1, 2};
    Mesh *obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 20, 12);
    for (int i = 0; i < 10; i++)
    {
        meshList.push_back(obj1);
    }
}

void CreateShaders()
{
    Shader *shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
}

void checkMouse()
{
    double xpos, ypos;
    glfwGetCursorPos(mainWindow.getWindow(), &xpos, &ypos);

    static float lastX = xpos;
    static float lastY = ypos;

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.5f;

    yaw += xoffset * sensitivity;
    pitch -= yoffset * sensitivity;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    // cout << xpos << " " << ypos << endl;
}

void CreateOBJ()
{
    Mesh *obj1 = new Mesh();
    bool loaded = obj1->CreateMeshFromOBJ("Models/Greed.obj");
    // bool loaded = obj1->CreateMeshFromOBJ("Models/suzanne.obj");
    if (loaded)
    {
        for (int i = 0; i < 10; i++)
        {
            meshList.push_back(obj1);
        }
    }
    else
    {
        std::cout << "Failed to load model" << std::endl;
    }
}
glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 0.0f);

int main()
{
    mainWindow = Window(WIDTH, HEIGHT, 3, 3);
    mainWindow.initialise();

    CreateOBJ();
    CreateShaders();
    CreateTriangle();
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);
    glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);
    // Texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("Texture/GreedAlbedo.png", &width, &height, &nrChannels, 0);
    // unsigned char *data = stbi_load("Texture/uvmap.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        cout << "Fail to load image" << endl;
    }

    stbi_image_free(data);
    // glBindTexture(GL_TEXTURE_2D, 0);

    GLuint uniformModel = 0, uniformProjection = 0, uniformView = 0;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

    float lastTime = static_cast<float>(glfwGetTime());

    // Loop until window closed
    while (!mainWindow.getShouldClose())
    {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Get + Handle user input events
        glfwPollEvents();

        checkMouse();

        glm::vec3 direction;
        direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        cameraDirection = glm::normalize(direction);
        cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W))
        {
            cameraPos += cameraDirection * deltaTime * 2.0f;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S))
        {
            cameraPos -= cameraDirection * deltaTime * 2.0f;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A))
        {
            cameraPos -= cameraRight * deltaTime * 2.0f;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D))
        {
            cameraPos += cameraRight * deltaTime * 2.0f;
        }

        cameraPos.y = 0;

        // Clear window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw here
        shaderList[0].UseShader();
        uniformModel = shaderList[0].GetUniformLocation("model");
        uniformView = shaderList[0].GetUniformLocation("view");
        uniformProjection = shaderList[0].GetUniformLocation("projection");

        glm::vec3 pyramidPositions[] =
            {
                glm::vec3(0.0f, 0.0f, -2.5f),
                glm::vec3(2.0f, 5.0f, -15.0f),
                glm::vec3(-1.5f, -2.2f, -2.5f),
                glm::vec3(-3.8f, -2.0f, -12.3f),
                glm::vec3(2.4f, -0.4f, -3.5f),
                glm::vec3(-1.7f, 3.0f, -7.5f),
                glm::vec3(1.3f, -2.0f, -2.5f),
                glm::vec3(1.5f, 2.0f, -2.5f),
                glm::vec3(1.5f, 0.2f, -1.5f),
                glm::vec3(-1.3f, 1.0f, -1.5f)};

        glm::mat4 view(1.0f);

        glm::mat4 cameraPosMat(1.0f);
        cameraPosMat[3][0] = -cameraPos.x;
        cameraPosMat[3][1] = -cameraPos.y;
        cameraPosMat[3][2] = -cameraPos.z;

        glm::mat4 cameraRotateMat(1.0f);
        cameraRotateMat[0] = glm::vec4(cameraRight.x, cameraUp.x, -cameraDirection.x, 0.0f);
        cameraRotateMat[1] = glm::vec4(cameraRight.y, cameraUp.y, -cameraDirection.y, 0.0f);
        cameraRotateMat[2] = glm::vec4(cameraRight.z, cameraUp.z, -cameraDirection.z, 0.0f);

        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);

        glUniform3fv(shaderList[0].GetUniformLocation("lightColour"), 1, (GLfloat *)&lightColour);
        // Object
        for (int i = 0; i < 1; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pyramidPositions[i]);
            float randomang = (rand() % 360);
            model = glm::rotate(model, glm::radians(randomang * i), glm::vec3(1.0f , 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f));
            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            //light
            glUniform3fv(shaderList[0].GetUniformLocation("viewPos"), 1, (GLfloat *)&cameraPos);
            glUniform3fv(shaderList[0].GetUniformLocation("lightPos"), 1, (GLfloat *)&lightPos);
            meshList[i]->RenderMesh();
            
        }
        glUseProgram(0);
        // end draw
        
        
        // magic word - SAKURA

        mainWindow.swapBuffers();
    }

    return 0;
}