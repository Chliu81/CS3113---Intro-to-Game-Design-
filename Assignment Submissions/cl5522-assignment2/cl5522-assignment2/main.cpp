#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char PLAYER_SPRITE_FILEPATH[] = "vic2.png";

const float MINIMUM_COLLISION_DISTANCE = 1.0f;

SDL_Window* display_window;
bool game_is_running = true;
bool is_growing = true;

ShaderProgram program;
glm::mat4 view_matrix, model_matrix, projection_matrix, trans_matrix,
other_model_matrix, upperwall_model_matrix, lowerwall_model_matrix,
leftwall_model_matrix, rightwall_model_matrix, ball_model_matrix;

float previous_ticks = 0.0f;

GLuint player_texture_id;
GLuint other_texture_id;

glm::vec3 player_position = glm::vec3(3.6f, 0.0f, 0.0f);
glm::vec3 player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 other_position = glm::vec3(-3.6f, 0.0f, 0.0f);
glm::vec3 other_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 player_orientation = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 player_rotation = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 ball_movement = glm::vec3(1.0f, 0.0f, 0.0f);


glm::vec3 upperwall_position = glm::vec3(0.0f, 3.5f, 0.0f);
glm::vec3 lowerwall_position = glm::vec3(0.0f, -3.5f, 0.0f);

glm::vec3 rightwall_position = glm::vec3(7.3f, 0.0f, 0.0f);
glm::vec3 leftwall_position = glm::vec3(-7.3f, 0.0f, 0.0f);

float player_speed = 3.0f;  // move 1 unit per second
float other_speed = 3.0f;
float ball_speed = 2.0f;

#define LOG(argument) std::cout << argument << '\n'

const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const int TRIANGLE_RED = 1.0,
TRIANGLE_BLUE = 0.4,
TRIANGLE_GREEN = 0.4,
TRIANGLE_OPACITY = 1.0;

const float INIT_BALL_ANGLE = glm::radians(45.0);

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    display_window = SDL_CreateWindow("Hello, Textures!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(display_window);
    SDL_GL_MakeCurrent(display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    program.Load(V_SHADER_PATH, F_SHADER_PATH);

    model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::mat4(1.0f);
    upperwall_model_matrix = glm::mat4(1.0f);
    upperwall_model_matrix = glm::translate(upperwall_model_matrix, upperwall_position);
    lowerwall_model_matrix = glm::mat4(1.0f);
    lowerwall_model_matrix = glm::translate(lowerwall_model_matrix, lowerwall_position);
    rightwall_model_matrix = glm::mat4(1.0f);
    rightwall_model_matrix = glm::translate(rightwall_model_matrix, rightwall_position);
    leftwall_model_matrix = glm::mat4(1.0f);
    leftwall_model_matrix = glm::translate(leftwall_model_matrix, leftwall_position);
    ball_model_matrix = glm::mat4(1.0f);

    //other_position += other_movement;
    //player_position += player_movement;


    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    program.SetColor(TRIANGLE_RED, TRIANGLE_BLUE, TRIANGLE_GREEN, TRIANGLE_OPACITY);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);

    // enable blending
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 Uses distance formula.
 */
bool check_collision(glm::vec3& position_a, glm::vec3& position_b)
{
    float x_distance = fabs(position_a[0] - position_b[0]) - 0.5f;
    float y_distance = fabs(position_a[1] - position_b[1]) - 0.7f;
    if (x_distance < 0 && y_distance < 0) {
        return true;
    }
    else {
        return false;
    }

}

bool check_collision_upperlower(glm::vec3& position_a, glm::vec3& position_b)
{
    float y_distance = fabs(position_a[1] - position_b[1]) - 0.7f;
    if (y_distance <= 0) {
        return true;
    }
    else {
        return false;
    }

}

bool check_collision_sides(glm::vec3& position_a, glm::vec3& position_b)
{
    float x_distance = fabs(position_a[0] - position_b[0]) - 0.7f;
    if (x_distance <= 0) {
        LOG("Game is Closing");
        return true;
    }
    else {
        return false;
    }

}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    player_movement = glm::vec3(0.0f);
    other_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (check_collision_sides(ball_position, rightwall_position)) {
            LOG("Game Successfully Closed");
            game_is_running = false;
        }

        if (check_collision_sides(ball_position, leftwall_position)) {
            LOG("Game Successfully Closed");
            game_is_running = false;
        }

        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                // Move the player left
                break;

            case SDLK_RIGHT:
                // Move the player right
                //player_movement.x = 1.0f;
                break;

            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_UP] && !check_collision_upperlower(player_position, upperwall_position))
    {
        player_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && !check_collision_upperlower(player_position, lowerwall_position))
    {
        player_movement.y = -1.0f;
    }
    else if (check_collision_upperlower(player_position, upperwall_position) && player_position[1] > 0){
        player_movement.y = -1.0f;
    }
    else if (check_collision_upperlower(player_position, lowerwall_position) && player_position[1] < 0){
        player_movement.y = 1.0f;
    }

    if (key_state[SDL_SCANCODE_W] && !check_collision_upperlower(other_position, upperwall_position)) {
        other_movement.y = 1.0f;
    }

    else if (key_state[SDL_SCANCODE_S] && !check_collision_upperlower(other_position, lowerwall_position)) {
        other_movement.y = -1.0f;
    }
    else if (check_collision_upperlower(other_position, upperwall_position) && other_position[1] > 0) {
        other_movement.y = -1.0f;
    }
    else if (check_collision_upperlower(other_position, lowerwall_position) && other_position[1] < 0) {
        other_movement.y = 1.0f;
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(player_movement) > 1.0f)
    {
        player_movement = glm::normalize(player_movement);
    }
}


void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    // Add direction * units per second * elapsed time

    player_position += player_movement * player_speed * delta_time;
    model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, player_position);

    other_position += other_movement * other_speed * delta_time;
    other_model_matrix = glm::mat4(1.0f);
    other_model_matrix = glm::translate(other_model_matrix, other_position);

    if (check_collision_upperlower(ball_position, upperwall_position) && ball_movement.x > 0) {
        ball_movement = glm::vec3(1.0f, -1.0f, 0.0f);
    }
    else if (check_collision_upperlower(ball_position, upperwall_position) && ball_movement.x < 0) {
        ball_movement = glm::vec3(-1.0f, -1.0f, 0.0f);
    }

    else if (check_collision_upperlower(ball_position, lowerwall_position) && ball_movement.x > 0) {
        ball_movement = glm::vec3(1.0f, 1.0f, 0.0f);
    }
    else if (check_collision_upperlower(ball_position, lowerwall_position) && ball_movement.x < 0) {
        ball_movement = glm::vec3(-1.0f, 1.0f, 0.0f);
    }

    if (check_collision(ball_position, player_position)) {
        ball_movement = glm::vec3(-1.0f, -1.0f, 0.0f);
    }

    else if (check_collision(ball_position, other_position)) {
        ball_movement = glm::vec3(1.0f, 1.0f, 0.0f);
    }

    if (check_collision_sides(ball_position, rightwall_position)) {
        ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        game_is_running = false;
    }

    else if (check_collision_sides(ball_position, leftwall_position)) {
        ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
        game_is_running = false;
    }

    ball_position += ball_movement * ball_speed * delta_time;
    ball_model_matrix = glm::mat4(1.0f);
    ball_model_matrix = glm::translate(ball_model_matrix, ball_position);
   
}

void draw_object(glm::mat4& object_model_matrix, GLuint& object_texture_id)
{
    program.SetModelMatrix(object_model_matrix);
    //glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void Draw_object(glm::mat4& object_model_matrix) {
    program.SetModelMatrix(object_model_matrix);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] = {
        -0.3f, -0.5f, 0.3f, -0.5f, 0.3f, 0.5f,  // triangle 1
        -0.3f, -0.5f, 0.3f, 0.5f, -0.3f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);

    //glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    //glEnableVertexAttribArray(program.texCoordAttribute);

    // Bind texture
    draw_object(model_matrix, player_texture_id);
    draw_object(other_model_matrix, other_texture_id);

    // We disable two attribute arrays now
    glDisableVertexAttribArray(program.positionAttribute);
    //glDisableVertexAttribArray(program.texCoordAttribute);

    float vertices_upperlower[] = {
        -10.0f, -0.5f, 10.0f, -0.5f, -10.0f, 0.5f,  // triangle 1
        10.0f, 0.5f, 10.0f, -0.5f, -10.0f, 0.5f   // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_upperlower);
    glEnableVertexAttribArray(program.positionAttribute);

    Draw_object(upperwall_model_matrix);
    Draw_object(lowerwall_model_matrix);

    glDisableVertexAttribArray(program.positionAttribute);

    float vertices_sides[] = {
        -2.5f, -10.0f, 2.5f, -10.0f, -2.5f, 10.0f,  // triangle 1
        2.5f, 10.0f, 2.5f, -10.0f, -2.5f, 10.0f   // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_sides);
    glEnableVertexAttribArray(program.positionAttribute);

    Draw_object(leftwall_model_matrix);
    Draw_object(rightwall_model_matrix);

    glDisableVertexAttribArray(program.positionAttribute);

    float vertices_ball[] = {
        -0.2f, -0.2f, 0.2f, -0.2f, -0.2f, 0.2f,  // triangle 1
        0.2f, 0.2f, 0.2f, -0.2f, -0.2f, 0.2f   // triangle 2
    };

    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_ball);
    glEnableVertexAttribArray(program.positionAttribute);

    Draw_object(ball_model_matrix);

    glDisableVertexAttribArray(program.positionAttribute);

    SDL_GL_SwapWindow(display_window);
}

void shutdown() { LOG("Game Successfully Closed");  SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}