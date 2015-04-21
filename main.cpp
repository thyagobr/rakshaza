#include <SDL.h>

struct Backbuffer {
  void *memory;
  int width;
  int height;
} backbuffer;

void render(void *backbuffer)
{

}


int main(int argc, char *argv[])
{
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  bool game_running = true;
  backbuffer.width = 1280;
  backbuffer.height = 720;
  backbuffer.memory = malloc(1280 * 720 * 4);


  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    printf("SDL couldn't be initialized\n %s\n", SDL_GetError());
    return(100);
  }

  SDL_CreateWindowAndRenderer(1280, 720, SDL_WINDOW_RESIZABLE, &window, &renderer);
  if ((!window) || (!renderer))
  {
    printf("rakshaza: window creation failed.\n %s\n", SDL_GetError());
    return(101);
  }

  texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      1280, 720);

  while (game_running)
  {
    while (SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_QUIT:
          printf("Quitting.\n");
          game_running = false;
          break;
      }
    }
  }

  free(backbuffer.memory);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}