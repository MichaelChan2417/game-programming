#pragma once

#include <string>

void wait_and_exit();

void wait_and_exit(const std::string& message);


// write_chunk and read_chunk

void write_chunk(const std::string& filename, const std::vector<uint32_t>& color_indexs, const std::unordered_map<glm::u8vec4, uint32_t>& color_to_index);