#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

glm::mat4 player1Matrix;
float player1_speed = 2.0f; // Speed Increased
// Start at 0, 0, 0
glm::vec3 player1_position = glm::vec3(-5, 0, 0);
// Don¡¯t go anywhere (yet).
glm::vec3 player1_movement = glm::vec3(0, 0, 0);

glm::mat4 player2Matrix;
float player2_speed = 2.0f; // Speed Increased
// Start at 0, 0, 0
glm::vec3 player2_position = glm::vec3(5, 0, 0);
// Don¡¯t go anywhere (yet).
glm::vec3 player2_movement = glm::vec3(0, 0, 0);

glm::mat4 ballMatrix;
float ball_speed = 2.0f; // Speed Increased
// Start at 0, 0, 0
glm::vec3 ball_position = glm::vec3(0, 0, 0);
// Don¡¯t go anywhere (yet).
glm::vec3 ball_movement = glm::vec3(0, 0, 0);

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("PONG!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640, 480);
    
    program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    player1Matrix = glm::mat4(1.0f);
    player2Matrix = glm::mat4(1.0f);
    ballMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

}

void ProcessInput() {

    player1_movement = glm::vec3(0);
    player2_movement = glm::vec3(0);

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            gameIsRunning = false;
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;
            case SDLK_RIGHT:
                // Move the player right
                break;
            case SDLK_SPACE:
                ball_position.x = 0;
                ball_position.y = 0;
                ball_movement.x = 1.0f;
                ball_movement.y = 1.0f;
                break;
            }
            break; // SDL_KEYDOWN
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    /*if (keys[SDL_SCANCODE_A]) {
        player1_movement.x = -1.0f;
    }
    else if (keys[SDL_SCANCODE_D]) {
        player1_movement.x = 1.0f;
    }*/
    if (keys[SDL_SCANCODE_S]) {
        player1_movement.y = -1.0f;
    }
    else if (keys[SDL_SCANCODE_W]) {
        player1_movement.y = 1.0f;
    }
    if (glm::length(player1_movement) > 1.0f) {
        player1_movement = glm::normalize(player1_movement);
    }

    /*if (keys[SDL_SCANCODE_LEFT]) {
        player2_movement.x = -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        player2_movement.x = 1.0f;
    }*/
    if (keys[SDL_SCANCODE_DOWN]) {
        player2_movement.y = -1.0f;
    }
    else if (keys[SDL_SCANCODE_UP]) {
        player2_movement.y = 1.0f;
    }
    if (glm::length(player2_movement) > 1.0f) {
        player2_movement = glm::normalize(player2_movement);
    }

    if (glm::length(ball_movement) > 1.0f) {
        ball_movement = glm::normalize(ball_movement);
    }
}

float lastTicks = 0.0f;

//Box-to-Box collisions will happen with the players and the ball - only the x direction will be reversed.
//object 1 will always be the ball.
GLboolean B2BCollisionCheck(glm::vec3 obj1, glm::vec3 obj2, float w1, float w2, float h1, float h2) {
    float x1 = obj1.x;
    float y1 = obj1.y;
    float x2 = obj2.x;
    float y2 = obj2.y;

    float xdist = fabs(x2 - x1) - ((w1 + w2) / 2.0f);
    float ydist = fabs(y2 - y1) - ((h1 + h2) / 2.0f);
    if (xdist < 0 && ydist < 0) {
        return true;
    }
    else return false;
}

GLboolean topBottomCheck(glm::vec3 obj, float h, float top, float bottom) {
    float y = obj.y;

    if (y >= top - h || y <= bottom + h) {
        return true;
    }
    else return false;
}


void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    // Add (direction * units per second * elapsed time)


    //Player 1 configuration
    player1_position += player1_movement * player1_speed * deltaTime;
    if (player1_position.y > 2.75) { player1_position.y = 2.75; }
    if (player1_position.y < -2.75) { player1_position.y = -2.75; }

    //Player 2 configuration
    player2_position += player2_movement * player2_speed * deltaTime;
    if (player2_position.y > 2.75) { player2_position.y = 2.75; }
    if (player2_position.y < -2.75) { player2_position.y = -2.75; }

    //Ball configuration
    if (B2BCollisionCheck(ball_position, player1_position, 0.4, 1, 0.4, 2)) {
        ball_movement.x = -ball_movement.x;
    }
    if (B2BCollisionCheck(ball_position, player2_position, 0.4, 1, 0.4, 2)) {
        ball_movement.x = -ball_movement.x;
    }
    if (topBottomCheck(ball_position, 0.4, 3.75, -3.75)) {
        ball_movement.y = -ball_movement.y;
    }
    ball_position += ball_movement * ball_speed * deltaTime;
    if (ball_position.y > 5) { ball_position.y = 5; }
    if (ball_position.y < -5) { ball_position.y = -5; }


    modelMatrix = glm::mat4(1.0f);
    player1Matrix = glm::translate(modelMatrix, player1_position);
    player2Matrix = glm::translate(modelMatrix, player2_position);
    ballMatrix = glm::translate(modelMatrix, ball_position);
}

//void DrawObject(glm::mat4 modelMatrix) {
//    program.SetModelMatrix(modelMatrix);
//
//    float vertices[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f};
//    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
//
//    glDrawArrays(GL_TRIANGLES, 0, 6);
//
//    glDisableVertexAttribArray(program.positionAttribute);}
//
//void Render() {
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    DrawObject(player1Matrix);
//    DrawObject(player2Matrix);
//    DrawObject(ballMatrix);
//
//    SDL_GL_SwapWindow(displayWindow);
//}

void DrawPlayer(glm::mat4 modelMatrix) {
    program.SetModelMatrix(modelMatrix);

    float vertices[] = { -0.5, -1, 0.5, -1, 0.5, 1, -0.5, -1, 0.5, 1, -0.5, 1 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
}

void DrawBall(glm::mat4 modelMatrix) {
    program.SetModelMatrix(modelMatrix);

    float vertices[] = { -0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    DrawPlayer(player1Matrix);
    DrawPlayer(player2Matrix);
    DrawBall(ballMatrix);

    SDL_GL_SwapWindow(displayWindow);
}



void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
        
    }
    
    Shutdown();
    return 0;
}
