/*

Author: Charles Liu
Program: Entity CPP file

*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

Entity::Entity(){

	position = glm::vec3(0);
	velocity = glm::vec3(0.0f);
	model_matrix = glm::mat4(0.0f);

	movement = glm::vec3(0.0f);

	speed = 0;
	model_matrix = glm::mat4(1.0f);
}

Entity::~Entity() {
	delete[] animation_up;
	delete[] animation_down;
	delete[] animation_left;
	delete[] animation_right;
	delete[] walking;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram* program, GLuint texture_id, int index) {

	//Step 1: Calculate the UV location of the indexed frame

	float u_coord = (float)(index % animation_cols) / (float)animation_cols;
	float v_coord = (float)(index / animation_cols) / (float)animation_rows;

	//Step 2: Calculate its UV size

	float width = 1.0f / (float)animation_cols;
	float height = 1.0f / (float)animation_rows;

	//Step 3: Match the texture coordinates to the vertices
	float tex_coords[] =
	{
		u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
		u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
	};

	float vertices[] =
	{
		-0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
		-0.5, -0.5, 0.5, 0.5, -0.5, 0.5
	};

	//Step 4: Render

	glBindTexture(GL_TEXTURE_2D, texture_id);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::update(float delta_time, Entity* collidable_entities, int collidable_entity_count) {
	if (!is_active) return;

	collided_top = false;
	collided_bottom = false;
	collided_left = false;
	collided_right = false;
	
	if (animation_indices != NULL) {
		
		if (glm::length(acceleration.x) != 0) {

			animation_time += delta_time;
			float frames_per_second = (float)1 / SECONDS_PER_FRAME;
			
			
			if (animation_time >= frames_per_second) {
				animation_time = 0.0f;
				animation_index++;
				
				if (animation_index >= animation_frames) {
					animation_index = 0;
				}
			}
		}
	}

	//Character moves from left to right, so they need an initial velocity
	//velocity.x = movement.x * speed;

	//Now we add the rest of the gravity physics
	velocity += acceleration * delta_time;

	position.y += velocity.y * delta_time;
	check_collision_y(collidable_entities, collidable_entity_count);

	position.x += velocity.x * delta_time;
	check_collision_x(collidable_entities, collidable_entity_count);

	//Jump
	if (is_thrusting_up) {
		//Step 1: Immediately return the flag to its originial false state
		is_thrusting_up = false;
		//Step 2: The player now acquires an upward velocity
		//velocity.y += jumping_power;
		acceleration.y = thrusting_power;
		//LOG(acceleration.y);
	}
	else {
		acceleration.y = gravity_effect;
	}

	if (is_thrusting_left) {
		is_thrusting_left = false;
		acceleration.x = -thrusting_power;
	}
	else if (is_thrusting_right) {
		is_thrusting_right = false;
		acceleration.x = thrusting_power;
	}
	else {
		acceleration.x = 0;
	}

	model_matrix = glm::mat4(1.0f);
	model_matrix = glm::translate(model_matrix, position);
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count) {
	for (int i = 0; i < collidable_entity_count; i++) {
		Entity* collidable_entity = &collidable_entities[i];

		if (check_collision(collidable_entity)) {
			
			float y_distance = fabs(position.y - collidable_entity->position.y);
			float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
			if (velocity.y > 0){
				position.y -= y_overlap;
				velocity.y = 0;
				collided_top = true;
			}
			else if (velocity.y < 0) {
				position.y += y_overlap;
				velocity.y = 0;
				collided_bottom = true;
			}
		}
	}
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count) {
	for (int i = 0; i < collidable_entity_count; i++) {
		Entity* collidable_entity = &collidable_entities[i];
		if (check_collision(collidable_entity)) {
			float x_distance = fabs(position.x - collidable_entity->position.x);
			float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));
			if (velocity.x > 0) {
				position.x -= x_overlap+0.01f;
				velocity.x = 0;
				collided_right = true;
			}
			else if (velocity.x < 0) {
				position.x += x_overlap+0.01f;
				velocity.x = 0;
				collided_left = true;
			}
		}
	}
}

void Entity::render(ShaderProgram* program) {

	program->SetModelMatrix(model_matrix);
	if (animation_indices != NULL) {
		draw_sprite_from_texture_atlas(program, texture_id, animation_indices[animation_index]);
		return;
	}

	

	switch (type) 
	{

		case START_PLATFORM:
		{
			float vertices[] = { -0.5, -0.25, 0.5, -0.25, 0.5, 0.25,
						-0.5, -0.25, 0.5, 0.25, -0.5, 0.25 };
			float tex_coords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			/*float tex_coords[] = { 0.0, 0.5, 0.5, 0.5, 0.5, 0.0,
								0.0, 0.5, 0.5, 0.0, 0.0, 0.0 };*/

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);
			break;
		}
		case PLAYER:
		{
			float vertices_player[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
						-0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float tex_coords_player[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices_player);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords_player);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			break;
		}

		case OBSTACLE_SMALL:
		{
			float vertices_obs_small[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
						-0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float tex_coords_obs_small[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices_obs_small);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords_obs_small);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			break;
		}

		case OBSTACLE_BIG:
		{
			float vertices_obs_big[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
						-0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float tex_coords_obs_big[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices_obs_big);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords_obs_big);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			break;
		}

		case END_PLATFORM:
		{
			float vertices_end_plat[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5,
						-0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float tex_coords_end_plat[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices_end_plat);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords_end_plat);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			break;
		}

		case TEXT:
		{
			float vertices_text[] = { -0.25, -0.25, 0.25, -0.25, 0.25, 0.25,
						-0.25, -0.25, 0.25, 0.25, -0.25, 0.25 };
			float tex_coords_text[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,
								0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

			glBindTexture(GL_TEXTURE_2D, texture_id);

			glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices_text);
			glEnableVertexAttribArray(program->positionAttribute);
			glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords_text);
			glEnableVertexAttribArray(program->texCoordAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glDisableVertexAttribArray(program->positionAttribute);
			glDisableVertexAttribArray(program->texCoordAttribute);

			break;
		}

	}


}

bool const Entity::check_collision(Entity* other) {
	//If either entity is inactive, there shouldn't be any collision
	if (!is_active || !other->is_active) return false;

	float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
	float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

	bool result = x_distance < 0.0f && y_distance < 0.0f;

	if (other->type != START_PLATFORM && other->type != END_PLATFORM && result ) {
		hit_obstacle = true;
	}
	else if (other->type == END_PLATFORM && result) {
		is_landed_success = true;
	}

	return result;
}




