#include "load_asset.hpp"

#include <iostream>
#include <exception>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include "util.hpp"


void load_sprite() {
    static_assert(SPRITE_COUNT == sprite_files.size(), "Sprite count does not match the file count");

    glm::uvec2 image_size = glm::uvec2(0, 0);
    std::vector<glm::u8vec4> data;

    try {
        load_png("../assets/sprite.png", &image_size, &data, LowerLeftOrigin);
    } catch (const std::exception& ex) {
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
        std::unordered_map<glm::u8vec4, uint32_t> color_to_index;
        std::vector<uint32_t> color_indexs;
        color_to_index[glm::u8vec4(0x00, 0x00, 0x00, 0x00)] = 0;    // use [0, 0, 0, 0] as 0
        uint32_t cur_index = 1;
        for (uint32_t y = 0; y < 8; y++) {
            for (uint32_t x = 0; x < 8; x++) {
                glm::u8vec4 color = data[(i * 8 + y) * image_size.x + x];
                
                if (color_to_index.find(color) == color_to_index.end()) {
                    color_to_index[color] = cur_index;
                    cur_index++;
                }
                // check if the color count is more than 3
                if (color_to_index.size() > 4) {
                    wait_and_exit("Sprite has more than 3 colors");
                }
                // tile index storage
                color_indexs.push_back(color_to_index[color]);
            }
        }
        
        write_chunk(sprite_files[i], color_indexs, color_to_index);
    }
}

void load_player() {
    
    glm::uvec2 image_size = glm::uvec2(0, 0);
    std::vector<glm::u8vec4> data;

    try {
        load_png("../assets/player.png", &image_size, &data, LowerLeftOrigin);
    } catch (const std::exception& ex) {
        std::cerr << "Error loading player png " << ex.what() << std::endl;
    }

    std::cout << "Player loaded" << std::endl;
    std::cout << "Size: " << image_size.x << " " << image_size.y << std::endl;
    std::cout << "Data size: " << data.size() << std::endl;
    // print all pixels with int cast
    for (int i = 0; i < data.size(); i++) {
        std::cout << "Pixel " << i << ": " << (int)data[i].r << " " << (int)data[i].g << " " << (int)data[i].b << " " << (int)data[i].a << std::endl;
    }
    
}