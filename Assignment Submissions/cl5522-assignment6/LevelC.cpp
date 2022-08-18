#include "LevelC.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

const char GUARD_FILEPATH[] = "assets/asteroid.png";
const char SPRITESHEET_FILEPATH[] = "assets/starship.png";
const char GREEN_LASER_FILEPATH[] = "assets/green_laser.png";
const char RED_LASER_FILEPATH[] = "assets/red_laser.png";
const char TEXT_FILEPATH[] = "assets/text_sheet.png";

unsigned int LEVEL_DATAC[] =
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

LevelC::~LevelC()
{
    delete[] this->state.enemies;
    delete    this->state.player;
    delete    this->state.map;
    for (Entity* ele : state.bullet_vector) {
        delete ele;
    }
    Mix_FreeChunk(this->state.jump_sfx);
    Mix_FreeMusic(this->state.bgm);
}

void LevelC::initialise()
{
    //GLuint map_texture_id = Utility::load_texture("assets/tileset.png");
    //this->state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVEL_DATA, map_texture_id, 1.0f, 4, 1);
    text_texture_id = Utility::load_texture(TEXT_FILEPATH);

    state.next_scene_id = -1;


    // Existing
    state.player = new Entity();
    state.player->set_entity_type(PLAYER);
    state.player->set_position(glm::vec3(5.0f, -3.0f, 0.0f));
    state.player->set_movement(glm::vec3(0.0f));
    state.player->speed = 2.5f;
    state.player->set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    state.player->gravity_effect = 0.0f;
    state.player->texture_id = Utility::load_texture(SPRITESHEET_FILEPATH);

    //state.player->model_matrix = glm::scale(state.player->model_matrix, glm::vec3(2.0f, 1.0f, 1.0f));

    // Walking
    state.player->set_height(0.8f);
    state.player->set_width(0.8f);

    //thrusting
    state.player->thrusting_power = 5.0f;

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
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        state.enemies[i].set_entity_type(ENEMY);
        state.enemies[i].set_ai_type(GUARD);
        state.enemies[i].set_ai_state(IDLE);
        state.enemies[i].texture_id = enemy_texture_id;
        state.enemies[i].set_movement(glm::vec3(0.0f));
        state.enemies[i].speed = 1.0f;
        state.player->rotate_speed = glm::radians(3.0);
        state.enemies[i].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
        state.enemies[i].set_height(0.8f);
        state.enemies[i].set_width(0.8f);
    }

    state.enemies[0].set_position(glm::vec3(2.0f, -1.0f, 0.0f));
    state.enemies[1].set_position(glm::vec3(1.0f, -5.0f, 0.0f));
    state.enemies[2].set_position(glm::vec3(2.0f, -6.0f, 0.0f));
    state.enemies[3].set_position(glm::vec3(5.0f, -1.0f, 0.0f));
    state.enemies[4].set_position(glm::vec3(7.0f, -7.0f, 0.0f));

    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    state.jump_sfx = Mix_LoadWAV("assets/jump.wav");

    state.bgm = Mix_LoadMUS("assets/adventure1.mp3");
    Mix_PlayMusic(state.bgm, -1);
    Mix_VolumeMusic(7.0f);

}

void LevelC::update(float delta_time) {
    int len_bullet_vector = state.bullet_vector.size();

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        if (state.enemies[i].get_active_state()) {
            enemies_active = true;
        }
    }


    if (!state.player->get_active_state()) {
        state.next_scene_id = 5;
        return;
    }

    if (!enemies_active) {
        state.next_scene_id = 4;
        return;
    }

    enemies_active = false;

    this->state.player->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT);

    for (int i = 0; i < ENEMY_COUNT; ++i) {
        state.enemies[i].update(delta_time, state.player, state.player, 1);
    }

    for (int i = 0; i < len_bullet_vector; ++i) {
        this->state.bullet_vector[i]->update(delta_time, state.player, state.enemies, this->ENEMY_COUNT);
        if (this->state.bullet_vector[i]->calc_distance(state.player) > 4.0f) {
            state.bullet_vector[i]->deactivate();
        }
    }

    //if (this->state.player->get_position().y < -10.0f) state.next_scene_id = 4;
    //std::cout << state.enemies[0].get_position().x << std::endl;

}

void LevelC::render(ShaderProgram* program)
{
    int x = state.player->get_lives();
    std::string c_lives = std::to_string(x);
    Utility::draw_text(program, text_texture_id, c_lives, 0.3f, 0.1f, glm::vec3(1.0f, -1.0f, 0.0f));
    int len_bullet_vector = state.bullet_vector.size();
    for (int i = 0; i < len_bullet_vector; ++i) {
        this->state.bullet_vector[i]->render(program);
    }
    for (int i = 0; i < ENEMY_COUNT; i++) this->state.enemies[i].render(program);
    this->state.player->render(program);
}
