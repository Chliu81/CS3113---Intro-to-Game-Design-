/*

Author: Charles Liu
Program: CS 3113 Assignment 1

*/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION


#ifdef _WINDOWS

#include <GL/glew.h>

#endif

#define GL_GLEXT_PROTOTYPES 1

#include <stdio.h>


#include <SDL.h>
#include <SDL_opengl.h>

#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(statement) std::cout << statement <<'\n';


//Constants

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f;
const float BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0;
const int VIEWPORT_Y = 0;
const int VIEWPORT_WIDTH = WINDOW_WIDTH;
const int VIEWPORT_HEIGHT = WINDOW_HEIGHT;

//texture constants

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl";
const char F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char PLAYER_SPRITE[] = "vic2.png";
const char PLAYER_SPRITE2[] = "tank.png";

GLuint player_texture_id;
GLuint player_texture_id2;

SDL_Window* displayWindow;
bool game_is_running = true;

ShaderProgram program;

glm::mat4 view_matrix;
glm::mat4 model_matrix;
glm::mat4 model_matrix2;
glm::mat4 projection_matrix;

//load_texture() constants
const int NUMBER_OF_TEXTURES = 1; //to be generated, that is
const GLint LEVEL_OF_DETAIL = 0; //base image level; Level n is the nth mipmap reduction image
const GLint TEXTURE_BORDER = 0; //this value MUST be zero

//translate, rotate, scale constants

float delta_time = 0.0333f;
float z_rotate = 0.04f;
float previous_ticks = 0.0f;
float scale_x = 1.01f;
const float MILLISECONDS_IN_SECOND = 1000.0;
const float DEGREES_PER_SECOND = 90.0f;
float model1_x = 0.0f;
float model1_y = 0.0f;
float model2_rotate = 0.0f;

GLuint load_texture(const char* filepath) {

	//STEP 1: Loading the image file
	//Try to load the image file
	int width, height, number_of_components;
	unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

	//Quit if it fails
	if (image == NULL) {
		LOG("Unable to load image, make sure that the path is correct.");
		assert(false);
	}

	//STEP 2: Generating and binding a texture ID to our image
	GLuint textureID;
	glGenTextures(NUMBER_OF_TEXTURES, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(
		GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA,
		width, height,
		TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE,
		image
	);

	//STEP 3: Setting our texture filter parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//STEP 4: Releasing our file from memory and returning our texture id
	stbi_image_free(image);
	return textureID;
}


void initialise() {

	SDL_Init(SDL_INIT_VIDEO);

	displayWindow = SDL_CreateWindow("Assignment 1!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);

	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

	program.Load(V_SHADER_PATH, F_SHADER_PATH);

	//Initialise our view, model, and project matrices
	view_matrix = glm::mat4(1.0f); //Identity 4x4 matrix
	model_matrix = glm::mat4(1.0f); // Identity 4x4 matrix
	model_matrix2 = glm::mat4(1.0f); 
	projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f,
		3.75f, -1.0f, 1.0f); //Orthogonal means perpendicuar: our camera
							 //will be looking perpendicularly down to our triangle

	program.SetViewMatrix(view_matrix);
	program.SetProjectionMatrix(projection_matrix);

	glUseProgram(program.programID);

	glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);

	//enable blending so that the transparent parts of images are transparent

	player_texture_id = load_texture(PLAYER_SPRITE);
	player_texture_id2 = load_texture(PLAYER_SPRITE2);

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
			game_is_running = false;
		}
	}

}

void update() {

	model_matrix = glm::mat4(1.0f);
	model_matrix2 = glm::mat4(1.0f);
	float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
	float delta_time = ticks - previous_ticks;
	previous_ticks = ticks;
	

	model1_x += 1.0 * delta_time;
	model2_rotate += glm::radians(90.0) * delta_time;

	model_matrix = glm::translate(model_matrix, glm::vec3(model1_x, 0.0f, 0.0f));
	model_matrix2 = glm::rotate(model_matrix2, model2_rotate, glm::vec3(0.0f, 0.0f, 1.0f));

}

void render() {

	glClear(GL_COLOR_BUFFER_BIT);

	//Vertices
	float vertices[] = {
		-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, //triangle 1
		-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f //triangle 2
	};


	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	// Textures
	float texture_coordinates[] = {
		0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, //triangle 1
		0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f //triangle 2
	};

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
	glEnableVertexAttribArray(program.texCoordAttribute);

	//Bind texture
	program.SetModelMatrix(model_matrix);
	glBindTexture(GL_TEXTURE_2D, player_texture_id);
	glDrawArrays(GL_TRIANGLES, 0, 6); //we are now drawing 2 traingles, so we use 6 instead of 3

	// we disable two attribute arrays now

	program.SetModelMatrix(model_matrix2);
	glBindTexture(GL_TEXTURE_2D, player_texture_id2);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

	SDL_GL_SwapWindow(displayWindow);

}

void shutdown() {
	SDL_Quit();
}


int main(int argc, char* argv[]) {

	initialise();

	while (game_is_running) {
		process_input();
		update();
		render();
	}

	shutdown();
	return 0;

}

