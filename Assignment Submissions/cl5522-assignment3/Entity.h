/*

Author: Charles Liu
Program: CS 3113 Entity Header File

*/

#ifndef ENTITY_H
#define ENTITY_H

enum EntityType {START_PLATFORM, PLAYER, OBSTACLE_SMALL, OBSTACLE_BIG, END_PLATFORM, TEXT};

class Entity {
private:

	bool is_active = true;

	int* animation_right = NULL;
	int* animation_left = NULL;
	int* animation_up = NULL;
	int* animation_down = NULL;
	

	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec3 acceleration;
	

	float width = 1;
	float height = 1;

public:

	// Static Attributes
	static const int SECONDS_PER_FRAME = 4;
	static const int LEFT = 0, RIGHT = 1, UP = 2, DOWN = 3;


	// Translating
	glm::vec3 movement;
	float speed;


	// Existing
	GLuint texture_id;
	glm::mat4 model_matrix;
	EntityType type;

	//Animating
	int** walking = new int* [4]{
		animation_left, animation_right, animation_up, animation_down
	};

	
	int* animation_indices = NULL;

	int animation_frames = 0;
	int animation_index = 0;

	float animation_time = 0.0f;

	int animation_cols = 0;
	int animation_rows = 0;

	//Game
	bool is_landed_success = false;
	bool hit_obstacle = false;

	// Thrusting
	bool is_thrusting_up = false;
	float thrusting_power = 0.0f;
	float gravity_effect = -2.0f;
	bool is_thrusting_right = false;
	
	bool is_thrusting_left = false;

	//Colliding
	bool collided_top = false;
	bool collided_bottom = false;
	bool collided_left = false;
	bool collided_right = false;

	//Methods

	Entity();

	~Entity();

	void draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index);
	void update(float delta_time, Entity* collidable_entities, int collidable_entity_count);
	void render(ShaderProgram* program);

	void const check_collision_y(Entity* collidable_entities, int collidable_entity_count);
	void const check_collision_x(Entity* collidable_entites, int collidable_entity_count);
	bool const check_collision(Entity* other) ;

	void activate() { is_active = true; };
	void deactivate() { is_active = false; };

	glm::vec3 const get_position() const { return position; };

	glm::vec3 const getmovement() const { return movement; };
	glm::vec3 const get_velocity() const { return velocity; };

	void const set_position(glm::vec3 new_position) { position = new_position; };
	void const set_movement(glm::vec3 new_movement) { movement = new_movement; };
	void const set_velocity(glm::vec3 new_velocity) { velocity = new_velocity; };
	void const set_acceleration(glm::vec3 new_acceleration) { acceleration = new_acceleration; };
	void const set_width(float new_width) { width = new_width; };
	void const set_height(float new_height) { height = new_height; };

};

#endif // !ENTITY_H
