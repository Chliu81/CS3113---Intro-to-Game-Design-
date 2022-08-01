/*

Author: Charles Liu
Program: CS 3113 main for Rise of AI assigment

*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define PLATFORM_COUNT 12
#define ENEMY_COUNT 3

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
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

/**
 STRUCTS AND ENUMS
 */
struct GameState
{
    Entity* player;
    Entity* platforms;
    Entity* enemies;
    Entity* msg_fail;
    Entity* msg_success;
};

/**
 CONSTANTS
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
const char SPRITESHEET_FILEPATH[] = "assets/inch_worm.png";
const char PLATFORM_FILEPATH[] = "assets/ground.png";
const char FLYER_FILEPATH[] = "assets/flying_frog.png";
const char GUARD_FILEPATH[] = "assets/frog.png";
const char JUMPER_FILEPATH[] = "assets/jumping_frog.png";
const char TEXT_FILEPATH[] = "assets/text_sheet.png";
 
const int NUMBER_OF_TEXTURES = 1; // to be generated, that is
const GLint LEVEL_OF_DETAIL = 0;  // base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0;   // this value MUST be zero

const float PLATFORM_OFFSET = 5.0f;

/**
 VARIABLES
 */
GameState state;

SDL_Window* display_window;
bool game_is_running = true;

ShaderProgram program;
glm::mat4 view_matrix, projection_matrix;

float previous_ticks = 0.0f;
float accumulator = 0.0f;

/**
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

    // STEP 3: Setting our texture filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Setting our texture wrapping modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // the last argument can change depending on what you are looking for
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // STEP 5: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    display_window = SDL_CreateWindow("Charles' Rise of AI!",
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

    /**
     Platform stuff
     */
    GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    state.platforms = new Entity[PLATFORM_COUNT];

    for (int i = 0; i < PLATFORM_COUNT-1; i++)
    {
        state.platforms[i].set_entity_type(PLATFORM);
        state.platforms[i].texture_id = platform_texture_id;
        state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
        state.platforms[i].set_width(0.4f);
        state.platforms[i].update(0.0f, NULL, NULL, 0);
    }

    state.platforms[PLATFORM_COUNT - 1].set_entity_type(PLATFORM);
    state.platforms[PLATFORM_COUNT - 1].texture_id = platform_texture_id;
    state.platforms[PLATFORM_COUNT - 1].set_position(glm::vec3(3.0, 0.5, 0.0f));
    state.platforms[PLATFORM_COUNT - 1].set_width(0.4f);
    state.platforms[PLATFORM_COUNT - 1].update(0.0f, NULL, NULL, 0);



    /**
     Player stuff
     */

     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(-2.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = load_texture(SPRITESHEET_FILEPATH);

    //state.player->model_matrix = glm::scale(state.player->model_matrix, glm::vec3(2.0f, 1.0f, 1.0f));

    // Walking
    state.player->set_height(0.9f);
    state.player->set_width(0.9f);

    // Jumping
    state.player->jumping_power = 8.0f;

    /**
     Guard's stuff
     */
    GLuint enemy_texture_id = load_texture(GUARD_FILEPATH);

    state.enemies = new Entity[ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(3.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[0].set_height(0.8f);
    state.enemies[0].set_height(0.8f);

    /*
    
    Flyer stuff

    */

    GLuint flyer_texture_id = load_texture(FLYER_FILEPATH);

    state.enemies[1].set_entity_type(ENEMY);
    state.enemies[1].set_ai_type(FLYER);
    state.enemies[1].set_ai_state(PATROLLING);
    state.enemies[1].texture_id = flyer_texture_id;
    state.enemies[1].set_position(glm::vec3(3.0f, 3.0f, 0.0f));
    state.enemies[1].set_movement(glm::vec3(0.0f));
    state.enemies[1].set_acceleration(glm::vec3(0.0f));
    state.enemies[1].set_width(0.5f);
    state.enemies[1].set_height(0.5f);


    /*

    JUMPER stuff

   */

    GLuint jumper_texture_id = load_texture(JUMPER_FILEPATH);

    state.enemies[2].set_entity_type(ENEMY);
    state.enemies[2].set_ai_type(JUMPER);
    state.enemies[2].set_ai_state(IDLE);
    state.enemies[2].texture_id = jumper_texture_id;
    state.enemies[2].set_position(glm::vec3(3.0f, 3.0f, 0.0f));
    state.enemies[2].set_movement(glm::vec3(0.0f));
    state.enemies[2].speed = 2.0f;
    state.enemies[2].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[2].jumping_power = 6.0f;
    state.enemies[2].set_height(0.8f);
    state.enemies[2].set_height(0.8f);

    //Fail Message
    GLuint fail_msg_texture_id = load_texture(TEXT_FILEPATH);
    state.msg_fail = new Entity[8];
    for (int i = 0; i < 8; ++i) {
        state.msg_fail[i].texture_id = fail_msg_texture_id;
        state.msg_fail[i].animation_cols = 16;
        state.msg_fail[i].animation_rows = 16;
        state.msg_fail[i].animation_indices = new int [8] {'Y', 'O', 'U', ' ', 'L', 'O', 'S', 'E'};
        state.msg_fail[i].animation_index = i;
        state.msg_fail[i].set_position(glm::vec3(-3.5f + i, 2.0f, 0.0f));
        state.msg_fail[i].update(FIXED_TIMESTEP, state.player, NULL, 0);
    }


    //Success Message

    GLuint success_msg_texture_id = load_texture(TEXT_FILEPATH);
    state.msg_success = new Entity[7];
    for (int i = 0; i < 7; ++i) {
        state.msg_success[i].texture_id = success_msg_texture_id;
        state.msg_success[i].animation_cols = 16;
        state.msg_success[i].animation_rows = 16;
        state.msg_success[i].animation_indices = new int [7] {'Y', 'O', 'U', ' ', 'W', 'I', 'N' };
        state.msg_success[i].animation_index = i;
        //state.msg_success[i].type = TEXT;
        state.msg_success[i].set_position(glm::vec3(-3.0f + i, 2.0f, 0.0f));
        state.msg_success[i].update(FIXED_TIMESTEP, state.player, NULL, 0);
    }

    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                game_is_running = false;
                break;

            case SDLK_SPACE:
                // Jump
                if (state.player->collided_bottom)
                {
                    state.player->is_jumping = true;
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_A])
    {
        state.player->movement.x = -1.0f;
        state.player->animation_indices = state.player->walking[state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_D])
    {
        state.player->movement.x = 1.0f;
        state.player->animation_indices = state.player->walking[state.player->RIGHT];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(state.player->movement) > 1.0f)
    {
        state.player->movement = glm::normalize(state.player->movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - previous_ticks;
    previous_ticks = ticks;

    delta_time += accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->update(FIXED_TIMESTEP, state.player, state.enemies, ENEMY_COUNT);
        for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);
        state.player->update(FIXED_TIMESTEP, state.player, state.platforms, PLATFORM_COUNT);

        delta_time -= FIXED_TIMESTEP;
    }

    accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    if (!state.player->get_active_state()) {
        for (int i = 0; i < 8; i++) state.msg_fail[i].render(&program);
    }

    else if (!state.enemies[0].get_active_state() 
        && !state.enemies[1].get_active_state() 
        && !state.enemies[2].get_active_state()) {
        for (int i = 0; i < 7; i++) state.msg_success[i].render(&program);
    }

    state.player->render(&program);

    for (int i = 0; i < PLATFORM_COUNT; i++) state.platforms[i].render(&program);
    for (int i = 0; i < ENEMY_COUNT; i++) state.enemies[i].render(&program);


    SDL_GL_SwapWindow(display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] state.platforms;
    delete[] state.enemies;
    delete    state.player;

}

/**
 DRIVER GAME LOOP
 */
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