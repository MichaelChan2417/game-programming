#pragma once

#include <array>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "PPU466.hpp"

void wait_and_exit();

void wait_and_exit(const std::string &message);

// write_chunk and read_chunk
void write_chunk(const std::string &filename,
                 const std::vector<uint32_t> &color_indexs,
                 const std::vector<glm::u8vec4> &colors);

void read_chunk(const std::string &filename, std::array<uint8_t, 8> &bit0s,
                std::array<uint8_t, 8> &bit1s,
                std::vector<glm::u8vec4> &colors);

PPU466::Tile mirror_tile(PPU466::Tile &tile);
