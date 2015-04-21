#include <SDL.h>
#include <time.h>

Uint64 performance_frequency;
Uint64 last_counter;
Uint64 end_counter;

struct Backbuffer {
  void *memory;
  int width;
  int height;
} backbuffer;

void render()
{

  Uint32 *pixel = (Uint32*) backbuffer.memory;
  for (int h = 0; h < backbuffer.height; ++h)
  {
    for (int w = 0; w < backbuffer.width; ++w)
    {
      *pixel++ = 0x912CEE;
      // *pixel++ = rand() % 4294967296;
    }
  } 
}


int main(int argc, char *argv[])
{
  performance_frequency = SDL_GetPerformanceFrequency();
  SDL_Event event;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  bool game_running = true;
  backbuffer.width = 1280;
  backbuffer.height = 720;
  backbuffer.memory = malloc(backbuffer.width * backbuffer.height * 4);


  if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
  {
    printf("SDL couldn't be initialized\n %s\n", SDL_GetError());
    return(100);
  }

  SDL_CreateWindowAndRenderer(backbuffer.width, backbuffer.height, SDL_WINDOW_RESIZABLE, &window, &renderer);
  if ((!window) || (!renderer))
  {
    printf("rakshaza: window creation failed.\n %s\n", SDL_GetError());
    return(101);
  }

  texture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      backbuffer.width, backbuffer.height);

  while (game_running)
  {
    last_counter = SDL_GetPerformanceCounter();
    render();
    SDL_UpdateTexture(texture, NULL, backbuffer.memory, backbuffer.width * sizeof(Uint32));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    while (SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_QUIT:
          printf("Quitting.\n");
          game_running = false;
          break;
        case SDL_WINDOWEVENT:
          {
          switch(event.window.event)
          {
            case SDL_WINDOWEVENT_RESIZED:
              {
                backbuffer.width = event.window.data1;
                backbuffer.height = event.window.data2;
                free(backbuffer.memory);
                backbuffer.memory = malloc(backbuffer.width * backbuffer.height * 4);
              } break;
          }
          } break;
      }
    }
    end_counter = SDL_GetPerformanceCounter();
    printf("ms per frame: %f\n", (((float) (end_counter - last_counter) * 1000.0f) / (float) performance_frequency));
    last_counter = end_counter;
  }

  free(backbuffer.memory);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
