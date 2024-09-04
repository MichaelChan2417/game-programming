#include "PlayMode.hpp"

#include <cstdint>

// for the GL_ERRORS() macro:
#include "PPU466.hpp"
#include "gl_errors.hpp"

#include "load_asset.hpp"
#include "load_save_png.hpp"
#include "util.hpp"

// for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

bool PlayMode::handle_event(SDL_Event const &evt,
                            glm::uvec2 const &window_size) {

  if (evt.type == SDL_KEYDOWN) {
    if (evt.key.keysym.sym == SDLK_LEFT) {
      left.downs += 1;
      left.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_RIGHT) {
      right.downs += 1;
      right.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_UP) {
      up.downs += 1;
      up.pressed = true;
      return true;
    } else if (evt.key.keysym.sym == SDLK_DOWN) {
      down.downs += 1;
      down.pressed = true;
      return true;
    }
  } else if (evt.type == SDL_KEYUP) {
    if (evt.key.keysym.sym == SDLK_LEFT) {
      left.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_RIGHT) {
      right.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_UP) {
      up.pressed = false;
      return true;
    } else if (evt.key.keysym.sym == SDLK_DOWN) {
      down.pressed = false;
      return true;
    }
  }

  return false;
}

PlayMode::PlayMode() {
  // parse & load sprites
  parse_sprite();
  load_sprites();

  // deal with background
  parse_and_load_background();
}

PlayMode::~PlayMode() {}

void PlayMode::update(float elapsed) {
  constexpr float PlayerSpeed = 30.0f;
  if (left.pressed)
    player_at.x -= PlayerSpeed * elapsed;
  if (right.pressed)
    player_at.x += PlayerSpeed * elapsed;
  if (down.pressed)
    player_at.y -= PlayerSpeed * elapsed;
  if (up.pressed)
    player_at.y += PlayerSpeed * elapsed;

  // reset button press counters:
  left.downs = 0;
  right.downs = 0;
  up.downs = 0;
  down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
  //--- set ppu state based on game state ---

  // background scroll: NO scroll in this game

  // player sprite:
  ppu.sprites[0].x = int8_t(player_at.x);
  ppu.sprites[0].y = int8_t(player_at.y);
  ppu.sprites[0].index = 4;
  ppu.sprites[0].attributes = 4;
  ppu.sprites[1].x = int8_t(player_at.x + 8);
  ppu.sprites[1].y = int8_t(player_at.y);
  ppu.sprites[1].index = 5;
  ppu.sprites[1].attributes = 4;
  ppu.sprites[2].x = int8_t(player_at.x);
  ppu.sprites[2].y = int8_t(player_at.y + 8);
  ppu.sprites[2].index = 6;
  ppu.sprites[2].attributes = 5;
  ppu.sprites[3].x = int8_t(player_at.x + 8);
  ppu.sprites[3].y = int8_t(player_at.y + 8);
  ppu.sprites[3].index = 7;
  ppu.sprites[3].attributes = 5;

  for (uint32_t i = 4; i < 63; i++) {
    ppu.sprites[i].x = 15;
    ppu.sprites[i].y = 240;
    ppu.sprites[i].index = 1;
    ppu.sprites[i].attributes = 1;
  }

  //--- actually draw ---
  ppu.draw(drawable_size);
}

void PlayMode::parse_and_load_background() {
  glm::uvec2 image_size = glm::uvec2(0, 0);
  std::vector<glm::u8vec4> data;

  try {
    load_png("../assets/background.png", &image_size, &data, LowerLeftOrigin);
  } catch (const std::exception &ex) {
    std::cerr << "Error loading background png " << ex.what() << std::endl;
    wait_and_exit();
  }

  // size should be 32x30
  if (image_size.x != 32 || image_size.y != 30) {
    wait_and_exit("Background size is not 32x30");
  }

  for (uint32_t i = 0; i < 30; i++) {
    for (uint32_t j = 0; j < 32; j++) {
      uint32_t index = i * 32 + j;
      uint32_t bindex = i * PPU466::BackgroundWidth + j;
      // get b color
      uint8_t b = data[index].b;
      if (b == 36) {
        ppu.background[bindex] = (0 << 8) | 0;
      } else if (b == 0) {
        ppu.background[bindex] = (1 << 8) | 1;
      } else if (b == 153) {
        ppu.background[bindex] = (2 << 8) | 2;
      } else if (b == 239) {
        ppu.background[bindex] = (3 << 8) | 3;
      }
    }
  }
}

void PlayMode::load_sprites() {
  uint32_t sprite_index = 0;
  for (uint32_t i = 0; i < SPRITE_COUNT; i++) {
    std::array<uint8_t, 8> bit0s;
    std::array<uint8_t, 8> bit1s;
    std::vector<glm::u8vec4> colors;
    read_chunk(sprite_files[i], bit0s, bit1s, colors);

    // store the sprite
    PPU466::Tile tile;
    for (uint32_t y = 0; y < 8; y++) {
      tile.bit0[y] = bit0s[y];
      tile.bit1[y] = bit1s[y];
    }
    ppu.tile_table[sprite_index++] = tile;
    if (i == PLAYER_BODY || i == PLAYER_HEAD) {
      ppu.tile_table[sprite_index++] = mirror_tile(tile);
    }

    // store the palette
    std::array<glm::u8vec4, 4> palette;
    for (uint32_t j = 0; j < 4; j++) {
      palette[j] = colors[j];
    }
    ppu.palette_table[i] = palette;

    // print color infos
    std::cout << "Pallete " << i << " loading ";
    for (uint32_t j = 0; j < 4; j++) {
      std::cout << "colors " << j << ": " << int(colors[j].r) << ","
                << int(colors[j].g) << "," << int(colors[j].b) << ","
                << int(colors[j].a) << " ";
    }
    std::cout << std::endl;
  }
}
