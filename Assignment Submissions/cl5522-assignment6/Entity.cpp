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
#include "Entity.h"

Entity::Entity()
{
    position     = glm::vec3(0.0f);
    velocity     = glm::vec3(0.0f);
    acceleration = glm::vec3(0.0f);
    
    movement = glm::vec3(0.0f);
    
    speed = 0;
    model_matrix = glm::mat4(1.0f);
}

Entity::~Entity()
{
    delete [] animation_up;
    delete [] animation_down;
    delete [] animation_left;
    delete [] animation_right;
    delete [] walking;
}

void Entity::draw_sprite_from_texture_atlas(ShaderProgram *program, GLuint texture_id, int index)
{
    // Step 1: Calculate the UV location of the indexed frame
    float u_coord = (float) (index % animation_cols) / (float) animation_cols;
    float v_coord = (float) (index / animation_cols) / (float) animation_rows;
    
    // Step 2: Calculate its UV size
    float width = 1.0f / (float) animation_cols;
    float height = 1.0f / (float) animation_rows;
    
    // Step 3: Just as we have done before, match the texture coordinates to the vertices
    float tex_coords[] =
    {
        u_coord, v_coord + height, u_coord + width, v_coord + height, u_coord + width, v_coord,
        u_coord, v_coord + height, u_coord + width, v_coord, u_coord, v_coord
    };
    
    float vertices[] =
    {
        -0.5, -0.5, 0.5, -0.5,  0.5, 0.5,
        -0.5, -0.5, 0.5,  0.5, -0.5, 0.5
    };
    
    // Step 4: And render
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::activate_ai(Entity *player)
{
    switch (ai_type)
    {
        case WALKER:
            ai_walker();
            break;
            
        case GUARD:
            ai_guard(player);
            break;
            
        default:
            break;
    }
}

void Entity::ai_walker()
{
    movement = glm::vec3(-1.0f, 0.0f, 0.0f);
}

void Entity::ai_guard(Entity *player)
{
    switch (ai_state) {
        case IDLE:
            if (glm::distance(position, player->position) < 6.0f) ai_state = WALKING;
            break;
            
        case WALKING:
            if (position.x > player->get_position().x) {
                movement.x = -1.0f;
            } else {
                movement.x = 1.0f;
            }

            if (position.y > player->get_position().y) {
                movement.y = -1.0f;
            }
            else {
                movement.y = 1.0f;
            }
            break;
            
        case ATTACKING:
            break;
            
        default:
            break;
    }
}

void Entity::update(float delta_time, Entity *player, Entity *objects, int object_count)
{

    //if (entity_type == PLAYER)std::cout << lives << std::endl;

    if (lives <= 0) {
        deactivate();
    }

    if (!is_active) return;

    acceleration = glm::vec3(0.0f);
    move_rotate = glm::radians(0.0f);
 
    collided_top    = false;
    collided_bottom = false;
    collided_left   = false;
    collided_right  = false;
    
    if (entity_type == ENEMY) activate_ai(player);
    
    if (animation_indices != NULL)
    {
        if (glm::length(movement) != 0)
        {
            animation_time += delta_time;
            float frames_per_second = (float) 1 / SECONDS_PER_FRAME;
            
            if (animation_time >= frames_per_second)
            {
                animation_time = 0.0f;
                animation_index++;
                
                if (animation_index >= animation_frames)
                {
                    animation_index = 0;
                }
            }
        }
    }

    switch (entity_type) {
    case PLAYER:
        //Thrust
        if (is_thrusting_up) {
            //Step 1: Immediately return the flag to its originial false state
            is_thrusting_up = false;
            //Step 2: The player now acquires an upward velocity
            //velocity.y += jumping_power;
            acceleration.y = thrusting_power;
            //LOG(acceleration.y);
        }
        else if (is_thrusting_down) {
            is_thrusting_down = false;
            acceleration.y = -thrusting_power;
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

        if (is_rotating_counter) {
            is_rotating_counter = false;
            move_rotate += rotate_speed;
            rotation += rotate_speed;
            if (rotation >= glm::radians(360.0)) {
                rotation -= glm::radians(360.0);
            }
        }
        else if (is_rotating_clock) {
            is_rotating_clock = false;
            move_rotate -= rotate_speed;
            rotation -= rotate_speed;
            if (rotation <= glm::radians(-360.0)) {
                rotation += glm::radians(360.0);
            }
        }
        // Our character moves from left to right, so they need an initial velocity
        //velocity.x = movement.x * speed;

        // Now we add the rest of the gravity physics
        velocity += acceleration * delta_time;

        position.y += velocity.y * delta_time;
        check_collision_y(objects, object_count);

        position.x += velocity.x * delta_time;
        check_collision_x(objects, object_count);

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, position);
        model_matrix = glm::rotate(model_matrix, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
        break;

    case ENEMY:
        velocity.x = movement.x * speed;
        velocity.y = movement.y * speed;

        position.y += velocity.y * delta_time;
        check_collision_y(objects, object_count);

        position.x += velocity.x * delta_time;
        check_collision_x(objects, object_count);

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, position);
        break;

    case GREEN_LASER:

 
        velocity.y = glm::cos(rotation) * speed;
        velocity.x = -glm::sin(rotation) * speed;

        position.y += velocity.y * delta_time;
        check_collision_y(objects, object_count);

        position.x += velocity.x * delta_time;
        check_collision_x(objects, object_count);

        model_matrix = glm::mat4(1.0f);
        model_matrix = glm::translate(model_matrix, position);

        break;

    case RED_LASER:
        break;

    }
   
}

float Entity::calc_distance(Entity* other) {
    float x_distance = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    float distance = glm::sqrt((x_distance * x_distance) + (y_distance * y_distance));
    return distance;
}

void const Entity::check_collision_y(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float y_distance = fabs(position.y - collidable_entity->position.y);
            float y_overlap = fabs(y_distance - (height / 2.0f) - (collidable_entity->height / 2.0f));
            if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                //deactivate();
                lives -= 1;
            }
            //else if (entity_type == ENEMY && collidable_entity->entity_type == PLAYER) {
                //collidable_entity->lives -= 1;
            //}
            else if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                collidable_entity->lives -= 1;
            }

            else if (entity_type == GREEN_LASER && collidable_entity->entity_type == ENEMY) {
                if (entity_type != BIG_ALIEN) {
                    collidable_entity->lives -= 1;
                }
                deactivate();
            }

            if (position.y - (height / 5.0f) < collidable_entity->position.y - collidable_entity->get_height() / 2.0f) {
                //if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                //    //deactivate();
                //    lives -= 1;
                //}
                //else if (entity_type == ENEMY && collidable_entity->entity_type == PLAYER) {
                //    collidable_entity->lives -= 1;
                //}
                position.y -= y_overlap + 0.5;
                velocity.y = 0;
                collided_top = true;
            }
            else if (position.y - (height / 5.0f) > collidable_entity->position.y + collidable_entity->get_height() / 2.0f) {
                //Adding Special killing collision
               /* if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                    collidable_entity->deactivate();
                }*/
                position.y += y_overlap+0.5;
                velocity.y = 0;
                collided_bottom = true;
            }
        }
    }
}

void const Entity::check_collision_x(Entity* collidable_entities, int collidable_entity_count)
{
    for (int i = 0; i < collidable_entity_count; i++)
    {
        Entity* collidable_entity = &collidable_entities[i];

        if (check_collision(collidable_entity))
        {
            float x_distance = fabs(position.x - collidable_entity->position.x);
            float x_overlap = fabs(x_distance - (width / 2.0f) - (collidable_entity->width / 2.0f));

            if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                //deactivate();
                lives -= 1;
            }
            else if (entity_type == ENEMY && collidable_entity->entity_type == PLAYER) {
                collidable_entity->lives -= 1;
            }
           

            else if (entity_type == GREEN_LASER && collidable_entity->entity_type == ENEMY) {
                if (entity_type != BIG_ALIEN) {
                    collidable_entity->lives -= 1;
                }
                deactivate();
            }

            if (position.x < collidable_entity->position.x) {
                //if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                //    //deactivate();
                //    lives -= 1;
                //}
                /*else if (entity_type == ENEMY && collidable_entity->entity_type == PLAYER) {
                    collidable_entity->lives -= 1;
                }*/
                position.x -= x_overlap+0.5;
                velocity.x = 0;
                collided_right = true;
            }
            else if (position.x > collidable_entity->position.x) {
                //if (entity_type == PLAYER && collidable_entity->entity_type == ENEMY) {
                //    //deactivate();
                //    lives -= 1;
                //}
                //else if (entity_type == ENEMY && collidable_entity->entity_type == PLAYER) {
                //    collidable_entity->lives -= 1;
                //}
                position.x += x_overlap+0.5;
                velocity.x = 0;
                collided_left = true;
            }
        }
    }
}

void Entity::render(ShaderProgram *program)
{
    if (!is_active) return;
    
    program->SetModelMatrix(model_matrix);
    
    if (animation_indices != NULL)
    {
        draw_sprite_from_texture_atlas(program, texture_id, animation_indices[animation_index]);
        return;
    }
    
    float vertices[]   = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float tex_coords[] = {  0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, tex_coords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool const Entity::check_collision(Entity *other) const
{
    // If we are checking with collisions with ourselves, this should be false
    if (other == this) return false;
    
    // If either entity is inactive, there shouldn't be any collision
    if (!is_active || !other->is_active) return false;
    
    float x_distance = fabs(position.x - other->position.x) - ((width  + other->width)  / 2.0f);
    float y_distance = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);
    
    return x_distance < 0.0f && y_distance < 0.0f;
}
