#include "Scene.h"

class LoseScreen : public Scene {
public:
	int ENEMY_COUNT = 0;

	GLuint main_menu_text_texture_id;

	~LoseScreen();

	void initialise() override;
	void update(float delta_time) override;
	void render(ShaderProgram* program) override;

};