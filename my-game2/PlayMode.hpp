#include "Mode.hpp"

#include <glm/glm.hpp>
#include <vector>

#include "Scene.hpp"

struct PlayMode : Mode {
  PlayMode();
  virtual ~PlayMode();

  // functions called by main loop:
  virtual bool handle_event(SDL_Event const &,
                            glm::uvec2 const &window_size) override;
  virtual void update(float elapsed) override;
  virtual void draw(glm::uvec2 const &drawable_size) override;

  //----- game state -----

  // input tracking:
  struct Button {
    uint8_t downs = 0;
    uint8_t pressed = 0;
  } left, right, down, up, mouse;

  // local copy of the game scene (so code can change it during gameplay):
  Scene scene;

  // camera:
  Scene::Camera *camera = nullptr;

  int enemy_index = 11;
};
