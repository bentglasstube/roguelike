#include "config.h"

Config::Config() : Game::Config() {
  graphics.title = "Roguelike";
  graphics.width = 256;
  graphics.height = 240;
  graphics.intscale = 3;
  graphics.fullscreen = false;
}
