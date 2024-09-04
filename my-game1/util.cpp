#include "util.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>

void wait_and_exit(const std::string &message) {
  std::cerr << message << std::endl;
  wait_and_exit();
}

void wait_and_exit() {
  std::cout << "Press ENTER to exit" << std::endl;
  std::cin.get();
  exit(-1);
}

void write_chunk(const std::string &filename,
                 const std::vector<uint32_t> &color_indexs,
                 const std::vector<glm::u8vec4> &colors) {
  assert(filename.size() == 4);
  std::string full_filename = "../assets/" + filename;

  std::ofstream out(full_filename, std::ios::binary);
  // write first 4 byte as name check
  out.write(filename.c_str(), 4);

  // first write colors in order
  for (auto &color : colors) {
    out.write(reinterpret_cast<const char *>(&color.r), sizeof(uint8_t));
    out.write(reinterpret_cast<const char *>(&color.g), sizeof(uint8_t));
    out.write(reinterpret_cast<const char *>(&color.b), sizeof(uint8_t));
    out.write(reinterpret_cast<const char *>(&color.a), sizeof(uint8_t));
  }
  // if the color_to_index is less than 4, pad with 0
  for (size_t i = colors.size(); i < 4; i++) {
    uint8_t zero = 0;
    out.write(reinterpret_cast<const char *>(&zero), sizeof(uint8_t) * 4);
  }

  // then for each tile, write the index
  std::array<uint8_t, 8> bit0s = {0};
  std::array<uint8_t, 8> bit1s = {0};

  for (uint32_t i = 0; i < color_indexs.size(); i++) {
    uint32_t index = color_indexs[i];
    uint32_t bit0 = index & 0x1;
    uint32_t bit1 = (index >> 1) & 0x1;
    bit0s[i / 8] |= bit0 << (i % 8);
    bit1s[i / 8] |= bit1 << (i % 8);
  }

  for (uint32_t i = 0; i < 8; i++) {
    out.write(reinterpret_cast<const char *>(&bit0s[i]), sizeof(uint8_t));
  }
  for (uint32_t i = 0; i < 8; i++) {
    out.write(reinterpret_cast<const char *>(&bit1s[i]), sizeof(uint8_t));
  }

  out.close();
}

void read_chunk(const std::string &filename, std::array<uint8_t, 8> &bit0s,
                std::array<uint8_t, 8> &bit1s,
                std::vector<glm::u8vec4> &colors) {
  assert(filename.size() == 4);
  std::string full_filename = "../assets/" + filename;
  std::ifstream in(full_filename, std::ios::binary);

  // first four bytes are name check
  char name[4];
  in.read(name, 4);
  std::string parsed_magic_name(name, 4);

  if (parsed_magic_name != filename) {
    wait_and_exit("File name does not match: expected - " + filename +
                  " got - " + parsed_magic_name);
  }

  // read color_to_index
  for (uint32_t i = 0; i < 4; i++) {
    glm::u8vec4 color;
    in.read(reinterpret_cast<char *>(&color.r), sizeof(uint8_t));
    in.read(reinterpret_cast<char *>(&color.g), sizeof(uint8_t));
    in.read(reinterpret_cast<char *>(&color.b), sizeof(uint8_t));
    in.read(reinterpret_cast<char *>(&color.a), sizeof(uint8_t));
    colors.push_back(color);
  }

  // read bit0s and bit1s
  for (uint32_t i = 0; i < 8; i++) {
    in.read(reinterpret_cast<char *>(&bit0s[i]), sizeof(uint8_t));
  }
  for (uint32_t i = 0; i < 8; i++) {
    in.read(reinterpret_cast<char *>(&bit1s[i]), sizeof(uint8_t));
  }
}

PPU466::Tile mirror_tile(PPU466::Tile &tile) {
  PPU466::Tile mirror;
  for (uint32_t i = 0; i < 8; i++) {
    mirror.bit0[i] = 0;
    mirror.bit1[i] = 0;
    for (uint32_t j = 0; j < 8; j++) {
      uint8_t bit0 = (tile.bit0[i] >> j) & 0x1;
      uint8_t bit1 = (tile.bit1[i] >> j) & 0x1;
      mirror.bit0[i] |= bit0 << (7 - j);
      mirror.bit1[i] |= bit1 << (7 - j);
    }
  }
  return mirror;
}
