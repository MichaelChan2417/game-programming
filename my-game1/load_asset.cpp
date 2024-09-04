#include "load_asset.hpp"

#include <exception>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "load_save_png.hpp"
#include "util.hpp"

int get_color_index_in_colors(const std::vector<glm::u8vec4> &colors,
                              const glm::u8vec4 &color) {
  for (uint32_t i = 0; i < colors.size(); i++) {
    if (colors[i] == color) {
      return i;
    }
  }
  return -1;
}

void parse_sprite() {
  if (SPRITE_COUNT != sprite_files.size()) {
    wait_and_exit("Sprite count does not match the file count");
  }

  glm::uvec2 image_size = glm::uvec2(0, 0);
  std::vector<glm::u8vec4> data;

  try {
    load_png("../assets/sprite.png", &image_size, &data, LowerLeftOrigin);
  } catch (const std::exception &ex) {
    std::cerr << "Error loading sprite png " << ex.what() << std::endl;
    wait_and_exit();
  }

  // sprite size checking
  if (image_size.x != 8 || image_size.y % 8 != 0) {
    wait_and_exit("Sprite size is not a multiple of 8");
  }
  // sprite count should match the size
  if (image_size.y / 8 != SPRITE_COUNT) {
    wait_and_exit("Sprite count does not match the size");
  }

  // load each 8x8 tile from bottom to top
  for (uint32_t i = 0; i < SPRITE_COUNT; i++) {
    // store unique colors
    std::vector<glm::u8vec4> colors;
    std::vector<uint32_t> color_indexs;
    colors.push_back(glm::u8vec4(0x00, 0x00, 0x00, 0x00));
    for (uint32_t y = 0; y < 8; y++) {
      for (uint32_t x = 0; x < 8; x++) {
        glm::u8vec4 color = data[(i * 8 + y) * image_size.x + x];

        if (get_color_index_in_colors(colors, color) == -1) {
          colors.push_back(color);
        }
        // check if the color count is more than 3
        if (colors.size() > 4) {
          wait_and_exit("Sprite has more than 3 colors");
        }
        // tile index storage
        color_indexs.push_back(get_color_index_in_colors(colors, color));
      }
    }

    assert(color_indexs.size() == 64);
    write_chunk(sprite_files[i], color_indexs, colors);
  }
}
