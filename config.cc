#include "config.h"

Config::Config() : Game::Config() {
  graphics.title = "Roguelike";
  graphics.width = 256;
  graphics.height = 240;
}
