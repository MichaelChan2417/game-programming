#pragma once

#include <string>
#include <vector>

// A sprite should be a 8x8(xN) image, where each 8x8 block is a tile
// each tile should contain no more than 3 non-transparent colors
// each tile index (from lower to upper) is list here
enum SpriteIndex {
  TARGET_FLAG = 0,
  PURE_BACKGROUND = 1,
  WALL = 2,
  ROAD = 3,
  PLAYER_BODY = 4,
  PLAYER_HEAD = 5,

  SPRITE_COUNT // keep this at the end as the count of all sprites
};

// asset pipeline storage name: should match above's order
static std::vector<std::string> sprite_files = {"flag", "back", "wall",
                                                "road", "ply0", "ply1"};

void parse_sprite();
