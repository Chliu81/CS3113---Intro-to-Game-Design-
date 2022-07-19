/*

Author:Charles Liu
Program: CS3113 Assignment 3: Lunar Lander

*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif


#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"


struct GameState {
    Entity* player;
    Entity* platforms;
    Entity* msg_fail;
    Entity* msg_success;
};

/*

Constants

*/

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

const char SPRITESHEET_FILEPATH[] = "assets/george_0.png";
const char PLATFORM_FILEPATH[] = "assets/platformPack_tile027.png";
const char TEXT_FILEPATH[] = "assets/font1.png";


const float MINIMUM_COLLISION_DISTANCE = 1.0f;
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

std::string str_fail = "MISSION FAILED";
int str_fail_len = 14;

std::string str_success = "MISSION SUCCESSFUL";
int str_success_len = 18;


/*

Variables

*/

GameState state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

/*

GENERAL FUNCTIONS

*/

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
    display_window = SDL_CreateWindow("Hello, Entities!",
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

    view_matrix = glm::mat4(1.0f);  // Defines the position (location and orientation) of the camera
    projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);  // Defines the characteristics of your camera, such as clip planes, field of view, projection method etc.

    program.SetProjectionMatrix(projection_matrix);
    program.SetViewMatrix(view_matrix);

    glUseProgram(program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //Platform stuff

    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT-1].type = END_PLATFORM;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(-1.5f, -2.35f, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].set_width(1.0f);
    state.platforms[PLATFORM_COUNT - 1].set_height(0.5f);
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, 0);

    for (int i = 0; i < PLATFORM_COUNT - 2; i++) {
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].type = START_PLATFORM;
        state.platforms[i].set_position(glm::vec3(i - 1.0f, -3.0f, 0.0f));
        state.platforms[i].set_width(1.0f);
        state.platforms[i].set_height(0.5f);
        state.platforms[i].update(0.0f, NULL, 0);
    }
    
    state.platforms[PLATFORM_COUNT - 2].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 2].type = OBSTACLE_SMALL;
    state.platforms[PLATFORM_COUNT - 2].set_position(glm::vec3(2.5f, -2.5f, 0.0f));
    state.platforms[PLATFORM_COUNT - 2].set_width(1.0f);
    state.platforms[PLATFORM_COUNT - 2].set_height(0.5f);
    state.platforms[PLATFORM_COUNT - 2].update(0.0f, NULL, 0);

    // Existing
    state.player = new Entity();
    state.player->type = PLAYER;
    state.player->set_position(glm::vec3(0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 1.0f;
    state.player->set_acceleration(glm::vec3(0.0f, -2.0f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);


    //Walking
    state.player->walking[state.player->LEFT] = new int[4]{ 1,5,9,13 };
    state.player->walking[state.player->RIGHT] = new int[4]{ 3,7,11,15 };
    state.player->walking[state.player->UP] = new int[4]{ 2, 6, 10, 14 };
    state.player->walking[state.player->DOWN] = new int[4]{ 0, 4, 8,  12 };
    
    state.player->animation_indices = state.player->walking[state.player->LEFT];
    state.player->animation_frames = 4;
    state.player->animation_index = 0;
    state.player->animation_time = 0.0f;
    state.player->animation_cols = 4;
    state.player->animation_rows = 4;
    state.player->set_height(0.9f);
    state.player->set_width(0.9f);

    //Jumping
    state.player->gravity_effect = -2.0f;
    state.player->thrusting_power = 3.0f;

    //Fail Message
    GLuint fail_msg_texture_id = load_texture(TEXT_FILEPATH);
    state.msg_fail = new Entity[str_fail_len];
    for (int i = 0; i < str_fail_len; ++i) {
        state.msg_fail[i].texture_id = fail_msg_texture_id;
        state.msg_fail[i].animation_cols = 16;
        state.msg_fail[i].animation_rows = 16;
        state.msg_fail[i].animation_indices = new int [str_fail_len] {'M', 'I', 'S', 'S', 'I', 'O', 'N', ' ', 'F', 'A', 'I', 'L', 'E', 'D'};
        state.msg_fail[i].animation_index = i;
        state.msg_fail[i].type = TEXT;
        state.msg_fail[i].set_position(glm::vec3((i*0.5f)-3.3f, 2.0f, 0.0f));
        state.msg_fail[i].update(0.0f, NULL, 0);
    }
    

    //Success Message

    GLuint success_msg_texture_id = load_texture(TEXT_FILEPATH);
    state.msg_success = new Entity[str_success_len];
    for (int i = 0; i < str_success_len; ++i) {
        state.msg_success[i].texture_id = success_msg_texture_id;
        state.msg_success[i].animation_cols = 16;
        state.msg_success[i].animation_rows = 16;
        state.msg_success[i].animation_indices = new int [str_success_len] {'M', 'I', 'S', 'S', 'I', 'O', 'N', ' ', 'S', 'U', 'C', 'C', 'E', 'S', 'S', 'F', 'U','L' };
        state.msg_success[i].animation_index = i;
        state.msg_success[i].type = TEXT;
        state.msg_success[i].set_position(glm::vec3((i * 0.5f) - 4.3f, 2.0f, 0.0f));
        state.msg_success[i].update(0.0f, NULL, 0);
    }

    //enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void process_input() {
    //Very Important: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));

    if (state.player->hit_obstacle) {

        state.player->set_velocity(glm::vec3(0.0f));
        state.player->set_acceleration(glm::vec3(0.0f));

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;


            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_q:
                    game_is_running = false;
                    break;

                default:
                    break;
                }

            default:
                break;

            }
        }

       return;
    }

    if (state.player->is_landed_success) {

        state.player->set_velocity(glm::vec3(0.0f));
        state.player->set_acceleration(glm::vec3(0.0f));

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                game_is_running = false;
                break;


            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_q:
                    game_is_running = false;
                    break;

                default:
                    break;
                }

            default:
                break;

            }
        }

        return;
    }



    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_q:
                    game_is_running = false;
                    break;

                case SDLK_SPACE:
                    //Jump
                    state.player->is_thrusting_up = true;
                    break;

                default:
                    break;
            }

        default:
            break;

        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A]) {
        state.player->animation_indices = state.player->walking[state.player->LEFT];
        state.player->is_thrusting_left = true;
    }
    else if (key_state[SDL_SCANCODE_D]) {
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
        state.player->is_thrusting_right = true;
    }

    if (key_state[SDL_SCANCODE_SPACE]) {
        state.player->is_thrusting_up = true;
    }

    if (glm::length(state.player->movement) > 1.0f) {
        state.player->movement = glm::normalize(state.player->movement);
    }

}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - previous_ticks; // the delta time is the difference from the last frame
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP) {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        state.player->update(FIXED_TIMESTEP, state.platforms, PLATFORM_COUNT);
        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;

    //LOG(state.player->is_landed_success);
    //LOG(state.player->hit_obstacle);

}




void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);

    if (state.player->hit_obstacle) {
        for (int i = 0; i < str_fail_len; i++) state.msg_fail[i].render(&program);
    }

    else if (state.player->is_landed_success) {
        for (int i = 0; i < str_success_len; i++) state.msg_success[i].render(&program);
    }

    SDL_GL_SwapWindow(display_window);
}

void shutdown() {

    SDL_Quit();

    delete[] state.platforms;
    delete state.player;
}


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