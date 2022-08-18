#include "Scene.h"

class LevelB : public Scene {
public:
    int ENEMY_COUNT = 5;
    bool enemies_active = true;
    GLuint text_texture_id;
    ~LevelB();

    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram* program) override;
};