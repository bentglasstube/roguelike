#include "game.h"

#include "config.h"
#include "title_screen.h"

int main(int, char**) {
  Game game(kConfig);
  game.loop(new TitleScreen());

  return 0;
}
