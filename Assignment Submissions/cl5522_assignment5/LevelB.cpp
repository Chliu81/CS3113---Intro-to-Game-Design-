#include "LevelB.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

const char GUARD_FILEPATH[] = "assets/frog.png";
const char SPRITESHEET_FILEPATH[] = "assets/inch_worm.png";

unsigned int LEVELB_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelB::initialise()
{
    GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);

    state.next_scene_id = -1;

    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
     // Existing
     //state.player = new Entity();
     //state.player->set_entity_type(PLAYER);
     //state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
     //state.player->set_movement(glm::vec3(0.0f));
     //state.player->speed = 2.5f;
     //state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
     //state.player->texture_id = Utility::load_texture("assets/george_0.png");
     //
     //// Walking
     //state.player->walking[state.player->LEFT]  = new int[4] { 1, 5, 9,  13 };
     //state.player->walking[state.player->RIGHT] = new int[4] { 3, 7, 11, 15 };
     //state.player->walking[state.player->UP]    = new int[4] { 2, 6, 10, 14 };
     //state.player->walking[state.player->DOWN]  = new int[4] { 0, 4, 8,  12 };

     //state.player->animation_indices = state.player->walking[state.player->RIGHT];  // start George looking left
     //state.player->animation_frames = 4;
     //state.player->animation_index  = 0;
     //state.player->animation_time   = 0.0f;
     //state.player->animation_cols   = 4;
     //state.player->animation_rows   = 4;
     //state.player->set_height(0.8f);
     //state.player->set_width(0.8f);

     // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.player->texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    //state.player->model_matrix = glm::scale(state.player->model_matrix, glm::vec3(2.0f, 1.0f, 1.0f));

    // Walking
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);

    // Jumping
    state.player->jumping_power = 8.0f;

    // Jumping
    state.player->jumping_power = 5.0f;

    /**
     Enemies' stuff */
     /*GLuint enemy_texture_id = Utility::load_texture("assets/soph.png");

     state.enemies = new Entity[this->ENEMY_COUNT];
     state.enemies[0].set_entity_type(ENEMY);
     state.enemies[0].set_ai_type(GUARD);
     state.enemies[0].set_ai_state(IDLE);
     state.enemies[0].texture_id = enemy_texture_id;
     state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
     state.enemies[0].set_movement(glm::vec3(0.0f));
     state.enemies[0].speed = 1.0f;
     state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));*/

     ///**
     // Guard's stuff
     // */
    GLuint enemy_texture_id = Utility::load_texture(GUARD_FILEPATH);

    state.enemies = new Entity[ENEMY_COUNT];
    state.enemies[0].set_entity_type(ENEMY);
    state.enemies[0].set_ai_type(GUARD);
    state.enemies[0].set_ai_state(IDLE);
    state.enemies[0].texture_id = enemy_texture_id;
    state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
    state.enemies[0].set_movement(glm::vec3(0.0f));
    state.enemies[0].speed = 1.0f;
    state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    state.enemies[0].set_height(0.8f);
    state.enemies[0].set_width(0.8f);


    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.jump_sfx = Mix_LoadWAV("assets/jump.wav");

    state.bgm = Mix_LoadMUS("assets/adventure1.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(7.0f);
}

void LevelB::update(float delta_time) { 
    if (!state.player->get_active_state()) {
        state.next_scene_id = 5;
        return;
    }
    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT, this->state.map);
    for (int i = 0; i < ENEMY_COUNT; ++i) state.enemies[i].update(delta_time, state.player, state.player, 1, this->state.map);
    if (this->state.player->get_position().y < -10.0f) state.next_scene_id = 3;
    //std::cout << state.enemies[0].get_position().x << std::endl;

}

void LevelB::render(ShaderProgram* program)
{
    for (int i = 0; i < ENEMY_COUNT; i++) this->state.enemies[i].render(program);
    this->state.map->render(program);
    this->state.player->render(program);
}
