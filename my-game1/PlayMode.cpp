#include "PlayMode.hpp"

#include <cstdint>
#include <random>

// for the GL_ERRORS() macro:
#include "gl_errors.hpp"

#include "load_asset.hpp"
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

  // tilemap gets recomputed every frame as some weird plasma thing:
  // NOTE: don't do this in your game! actually make a map or something :-)
  for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
    for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
      // TODO: make weird plasma thing
      ppu.background[x + PPU466::BackgroundWidth * y] = 1;
    }
  }

  // background scroll:
  // ppu.background_position.x = int32_t(-0.5f * player_at.x);
  // ppu.background_position.y = int32_t(-0.5f * player_at.y);

  // player sprite:
  ppu.sprites[0].x = int8_t(player_at.x);
  ppu.sprites[0].y = int8_t(player_at.y);
  ppu.sprites[0].index = 0;
  ppu.sprites[0].attributes = 0;

  // some other misc sprites:
  // for (uint32_t i = 1; i < 63; ++i) {
  // 	float amt = (i + 2.0f * background_fade) / 62.0f;
  // 	ppu.sprites[i].x = int8_t(0.5f * PPU466::ScreenWidth + std::cos( 2.0f *
  // M_PI * amt * 5.0f + 0.01f * player_at.x) * 0.4f * PPU466::ScreenWidth);
  // 	ppu.sprites[i].y = int8_t(0.5f * PPU466::ScreenHeight + std::sin( 2.0f *
  // M_PI * amt * 3.0f + 0.01f * player_at.y) * 0.4f * PPU466::ScreenWidth);
  // 	ppu.sprites[i].index = 32;
  // 	ppu.sprites[i].attributes = 6;
  // 	if (i % 2) ppu.sprites[i].attributes |= 0x80; //'behind' bit
  // }
  for (uint32_t i = 1; i < 63; i++) {
    ppu.sprites[i].x = 15;
    ppu.sprites[i].y = 240;
    ppu.sprites[i].index = 1;
    ppu.sprites[i].attributes = 1;
  }

  //--- actually draw ---
  ppu.draw(drawable_size);
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

    // store the palette
    std::array<glm::u8vec4, 4> palette;
    for (uint32_t j = 0; j < 4; j++) {
      palette[j] = colors[j];
    }
    ppu.palette_table[i] = palette;

    // print color infos
    std::cout << "Sprite " << i << " loading ";
    for (uint32_t j = 0; j < 4; j++) {
      std::cout << "colors " << j << ": " << int(colors[j].r) << ","
                << int(colors[j].g) << "," << int(colors[j].b) << ","
                << int(colors[j].a) << " ";
    }
    std::cout << std::endl;
  }
}
