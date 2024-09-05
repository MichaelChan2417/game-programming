#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "Mode.hpp"
#include "PPU466.hpp"

enum ObjectType { PLAYER };

struct Object {
  ObjectType type_;
};

struct PlayMode : Mode {
  PlayMode();
  virtual ~PlayMode();

  void load_sprites();
  void parse_and_load_background();

  // functions called by main loop:
  virtual bool handle_event(SDL_Event const &,
                            glm::uvec2 const &window_size) override;
  virtual void update(float elapsed) override;
  virtual void draw(glm::uvec2 const &drawable_size) override;

  //----- game state -----
  std::array<uint16_t, 32 * 30> default_background_;

  // input tracking:
  struct Button {
    uint8_t downs = 0;
    uint8_t pressed = 0;
  } left, right, down, up;

  // some weird background animation:
  float background_fade = 0.0f;

  // player position:
  glm::vec2 player_at = glm::vec2(0.0f);
  std::vector<std::vector<uint8_t>> backgrounds_{32, std::vector<uint8_t>(30, 0)};

  //----- drawing handled by PPU466 -----

  PPU466 ppu;
};
