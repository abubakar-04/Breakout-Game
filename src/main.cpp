// LearnOpenGL-style starter: window + context + render loop.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float paddleX = 350.0f;
float ballX = 390.0f;
float ballY = 300.0f;
float ballVelX = 280.0f;
float ballVelY = -280.0f;
bool gamePaused = false;

struct Brick
{
    float x;
    float y;
    float width;
    float height;
    bool active;
};

bool checkAABB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh);
void drawQuad(Shader &shader, unsigned int vao, float x, float y, float w, float h, float r, float g, float b);
void drawDigit(Shader &shader, unsigned int vao, int digit, float x, float y, float size, float thickness, float r, float g, float b);
void drawNumber(Shader &shader, unsigned int vao, int value, float x, float y, float size, float thickness, float r, float g, float b);

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

    const int brickRows = 5;
    const int brickCols = 10;
    const float brickWidth = 70.0f;
    const float brickHeight = 20.0f;
    const float brickGap = 8.0f;
    const float brickStartX = 15.0f;
    const float brickStartY = 60.0f;

    int score = 0;
    bool gameOver = false;
    bool gameWon = false;
    bool restartKeyWasDown = false;

    std::vector<Brick> bricks;
    bricks.reserve(brickRows * brickCols);
    for (int row = 0; row < brickRows; ++row)
    {
        for (int col = 0; col < brickCols; ++col)
        {
            Brick brick;
            brick.x = brickStartX + col * (brickWidth + brickGap);
            brick.y = brickStartY + row * (brickHeight + brickGap);
            brick.width = brickWidth;
            brick.height = brickHeight;
            brick.active = true;
            bricks.push_back(brick);
        }
    }

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

        std::ostringstream titleBuilder;
        titleBuilder << "Breakout Game | Score: " << score;
        if (gameWon)
        {
            titleBuilder << " | You Win! Press R to restart";
        }
        else if (gameOver)
        {
            titleBuilder << " | Game Over! Press R to restart";
        }
        glfwSetWindowTitle(window, titleBuilder.str().c_str());

        bool restartKeyDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (gamePaused && restartKeyDown && !restartKeyWasDown)
        {
            score = 0;
            gamePaused = false;
            gameOver = false;
            gameWon = false;
            paddleX = 350.0f;
            ballX = 390.0f;
            ballY = 300.0f;
            ballVelX = 260.0f;
            ballVelY = -260.0f;

            for (Brick &brick : bricks)
            {
                brick.active = true;
            }
        }
        restartKeyWasDown = restartKeyDown;

        processInput(window);

        glClearColor(0.08f, 0.08f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!gamePaused)
        {
            ballX += ballVelX * deltaTime;
            ballY += ballVelY * deltaTime;
        }

        if (!gamePaused && ballX <= 0.0f)
        {
            ballX = 0.0f;
            ballVelX = -ballVelX;
        }
        if (!gamePaused && ballX + ballSize >= static_cast<float>(SCREEN_WIDTH))
        {
            ballX = static_cast<float>(SCREEN_WIDTH) - ballSize;
            ballVelX = -ballVelX;
        }
        if (!gamePaused && ballY <= 0.0f)
        {
            ballY = 0.0f;
            ballVelY = -ballVelY;
        }
        if (!gamePaused && ballY + ballSize >= static_cast<float>(SCREEN_HEIGHT))
        {
            gamePaused = true;
            gameOver = true;
            ballVelX = 0.0f;
            ballVelY = 0.0f;
        }

        if (!gamePaused && checkAABB(ballX, ballY, ballSize, ballSize, paddleX, paddleY, paddleWidth, paddleHeight) && ballVelY > 0.0f)
        {
            ballY = paddleY - ballSize;

            float ballCenterX = ballX + ballSize * 0.5f;
            float paddleCenterX = paddleX + paddleWidth * 0.5f;
            float hit = (ballCenterX - paddleCenterX) / (paddleWidth * 0.5f);

            if (hit < -1.0f)
            {
                hit = -1.0f;
            }
            if (hit > 1.0f)
            {
                hit = 1.0f;
            }

            float baseSpeed = 500.0f;
            ballVelX = baseSpeed * hit;
            ballVelY = -std::abs(ballVelY);
        }

        bool hitBrickThisFrame = false;
        for (Brick &brick : bricks)
        {
            if (gamePaused)
            {
                break;
            }

            if (!brick.active)
            {
                continue;
            }

            if (checkAABB(ballX, ballY, ballSize, ballSize, brick.x, brick.y, brick.width, brick.height))
            {
                brick.active = false;
                score += 10;
                std::cout << "Score: " << score << std::endl;

                float ballCenterX = ballX + ballSize * 0.5f;
                float ballCenterY = ballY + ballSize * 0.5f;
                float brickCenterX = brick.x + brick.width * 0.5f;
                float brickCenterY = brick.y + brick.height * 0.5f;

                float dx = ballCenterX - brickCenterX;
                float dy = ballCenterY - brickCenterY;
                float overlapX = (ballSize * 0.5f + brick.width * 0.5f) - std::abs(dx);
                float overlapY = (ballSize * 0.5f + brick.height * 0.5f) - std::abs(dy);

                if (overlapX < overlapY)
                {
                    ballVelX = -ballVelX;
                }
                else
                {
                    ballVelY = -ballVelY;
                }

                hitBrickThisFrame = true;
                break;
            }
        }

        if (hitBrickThisFrame)
        {
            ballX += ballVelX * deltaTime;
            ballY += ballVelY * deltaTime;
        }

        if (!gamePaused)
        {
            bool anyBrickActive = false;
            for (const Brick &brick : bricks)
            {
                if (brick.active)
                {
                    anyBrickActive = true;
                    break;
                }
            }

            if (!anyBrickActive)
            {
                gamePaused = true;
                gameWon = true;
                ballVelX = 0.0f;
                ballVelY = 0.0f;
            }
        }

        rectangleShader.use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(paddleX, paddleY, 0.0f));
        model = glm::scale(model, glm::vec3(paddleWidth, paddleHeight, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(rectangleShader.ID, "spriteColor"), 0.93f, 0.76f, 0.25f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        for (const Brick &brick : bricks)
        {
            if (!brick.active)
            {
                continue;
            }

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(brick.x, brick.y, 0.0f));
            model = glm::scale(model, glm::vec3(brick.width, brick.height, 1.0f));
            glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniform3f(glGetUniformLocation(rectangleShader.ID, "spriteColor"), 0.88f, 0.34f, 0.33f);

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(ballX, ballY, 0.0f));
        model = glm::scale(model, glm::vec3(ballSize, ballSize, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(rectangleShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3f(glGetUniformLocation(rectangleShader.ID, "spriteColor"), 0.42f, 0.84f, 0.97f);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw score directly inside the game window using seven-segment digits.
        drawNumber(rectangleShader, VAO, score, 20.0f, 16.0f, 20.0f, 4.0f, 0.95f, 0.95f, 0.95f);

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

    if (gamePaused)
    {
        return;
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

bool checkAABB(float ax, float ay, float aw, float ah, float bx, float by, float bw, float bh)
{
    bool overlapX = ax < bx + bw && ax + aw > bx;
    bool overlapY = ay < by + bh && ay + ah > by;
    return overlapX && overlapY;
}

void drawQuad(Shader &shader, unsigned int vao, float x, float y, float w, float h, float r, float g, float b)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));
    model = glm::scale(model, glm::vec3(w, h, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shader.ID, "spriteColor"), r, g, b);

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawDigit(Shader &shader, unsigned int vao, int digit, float x, float y, float size, float thickness, float r, float g, float b)
{
    if (digit < 0 || digit > 9)
    {
        return;
    }

    const unsigned char masks[10] = {
        0x3F, // 0: a b c d e f
        0x06, // 1: b c
        0x5B, // 2: a b g e d
        0x4F, // 3: a b c d g
        0x66, // 4: f g b c
        0x6D, // 5: a f g c d
        0x7D, // 6: a f e d c g
        0x07, // 7: a b c
        0x7F, // 8: a b c d e f g
        0x6F  // 9: a b c d f g
    };

    const unsigned char mask = masks[digit];
    const float height = size * 1.6f;

    // Segment layout: a(top), b(upper-right), c(lower-right), d(bottom), e(lower-left), f(upper-left), g(middle)
    if (mask & (1 << 0))
    {
        drawQuad(shader, vao, x, y, size, thickness, r, g, b);
    }
    if (mask & (1 << 1))
    {
        drawQuad(shader, vao, x + size - thickness, y, thickness, height * 0.5f, r, g, b);
    }
    if (mask & (1 << 2))
    {
        drawQuad(shader, vao, x + size - thickness, y + height * 0.5f, thickness, height * 0.5f, r, g, b);
    }
    if (mask & (1 << 3))
    {
        drawQuad(shader, vao, x, y + height - thickness, size, thickness, r, g, b);
    }
    if (mask & (1 << 4))
    {
        drawQuad(shader, vao, x, y + height * 0.5f, thickness, height * 0.5f, r, g, b);
    }
    if (mask & (1 << 5))
    {
        drawQuad(shader, vao, x, y, thickness, height * 0.5f, r, g, b);
    }
    if (mask & (1 << 6))
    {
        drawQuad(shader, vao, x, y + height * 0.5f - thickness * 0.5f, size, thickness, r, g, b);
    }
}

void drawNumber(Shader &shader, unsigned int vao, int value, float x, float y, float size, float thickness, float r, float g, float b)
{
    if (value < 0)
    {
        value = 0;
    }

    std::string text = std::to_string(value);
    float cursorX = x;
    for (char c : text)
    {
        int digit = c - '0';
        drawDigit(shader, vao, digit, cursorX, y, size, thickness, r, g, b);
        cursorX += size + thickness * 2.0f;
    }
}
