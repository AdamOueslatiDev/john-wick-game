#include "game.h"

int main(int argc, char *argv[]) {
  GameContext ctx;
  SDL_Event ev;
  int last, current, dt;

  argc = argc;
  argv = argv;

  if (!initGame(&ctx, 0)) {
    fprintf(stderr, "initGame() failed.\n");
    return 1;
  }

  last = SDL_GetTicks();

  while (ctx.running) {
    current = SDL_GetTicks();
    dt = current - last;
    last = current;

    while (SDL_PollEvent(&ev))
      handleEvents(&ctx, &ev);

    updateGame(&ctx, dt);
    renderGame(&ctx);

    SDL_Delay(16);
  }

  cleanupGame(&ctx);
  return 0;
}
