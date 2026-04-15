// LearnOpenGL-style starter: window + context + render loop.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float paddleX = 350.0f;
float ballX = 390.0f;
float ballY = 300.0f;
float ballVelX = 260.0f;
float ballVelY = -260.0f;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        "Breakout Game",
        nullptr,
        nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    Shader rectangleShader("shaders/vertex.glsl", "shaders/fragment.glsl");

    float vertices[] = {
        // local-space unit quad
        0.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f};

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    const float paddleWidth = 100.0f;
    const float paddleHeight = 20.0f;
    const float paddleY = 560.0f;

    const float ballSize = 20.0f;

    rectangleShader.use();
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT), 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.08f, 0.08f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ballX += ballVelX * deltaTime;
        ballY += ballVelY * deltaTime;

        if (ballX <= 0.0f)
        {
            ballX = 0.0f;
            ballVelX = -ballVelX;
        }
        if (ballX + ballSize >= static_cast<float>(SCREEN_WIDTH))
        {
            ballX = static_cast<float>(SCREEN_WIDTH) - ballSize;
            ballVelX = -ballVelX;
        }
        if (ballY <= 0.0f)
        {
            ballY = 0.0f;
            ballVelY = -ballVelY;
        }
        if (ballY + ballSize >= static_cast<float>(SCREEN_HEIGHT))
        {
            ballX = 390.0f;
            ballY = 300.0f;
            ballVelY = -260.0f;
        }

        rectangleShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(paddleX, paddleY, 0.0f));
        model = glm::scale(model, glm::vec3(paddleWidth, paddleHeight, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(rectangleShader.ID, "spriteColor"), 0.93f, 0.76f, 0.25f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(ballX, ballY, 0.0f));
        model = glm::scale(model, glm::vec3(ballSize, ballSize, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(rectangleShader.ID, "spriteColor"), 0.42f, 0.84f, 0.97f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    float paddleSpeed = 450.0f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        paddleX -= paddleSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        paddleX += paddleSpeed;
    }

    if (paddleX < 0.0f)
    {
        paddleX = 0.0f;
    }
    if (paddleX > static_cast<float>(SCREEN_WIDTH) - 100.0f)
    {
        paddleX = static_cast<float>(SCREEN_WIDTH) - 100.0f;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)window;
    glViewport(0, 0, width, height);
}
