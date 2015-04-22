#include <SDL.h>
#include <time.h>

typedef short int16;

Uint64 performance_frequency;
Uint64 last_counter;
Uint64 end_counter;

#define PI 3.14159265358979f

struct Backbuffer {
  void *memory;
  int width;
  int height;
} backbuffer;

void audio_init(Uint32 samples_per_second, Uint32 buffer_size)
{
  SDL_AudioSpec audio_settings = {0};
  audio_settings.freq = samples_per_second;
  audio_settings.format = AUDIO_S16;
  audio_settings.channels = 2;    /* 1 = mono, 2 = stereo */
  audio_settings.samples = buffer_size;

  /* Open the audio device, forcing the desired format */
  if (SDL_OpenAudio(&audio_settings, NULL) < 0) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
  }
  else 
  {
    printf("Audio systems operational.\n");
  }
}

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

  // 32 bits for stereo sample: 16 for left, 16 for right
  // we need this out every second
  int sampling_rate = 48000;
  int tone_frequency_hz = 256;
  int number_of_periods = 0;
  int wave_period = sampling_rate / tone_frequency_hz;
  int bytes_per_sample = sizeof(short) * 2;
  int max_bytes_to_write = 48000 * bytes_per_sample;
  void* sound_buffer;
  bool sound_playing = false;
  audio_init(sampling_rate, (sampling_rate * bytes_per_sample) / 60);


  while (game_running)
  {
    last_counter = SDL_GetPerformanceCounter();

    int bytes_to_write = max_bytes_to_write - SDL_GetQueuedAudioSize(1);
    if (bytes_to_write)
    {
      sound_buffer = malloc(bytes_to_write);
      int16* sound_buffer_pointer = (int16*) sound_buffer;
      for (int i = 0; i < (bytes_to_write / bytes_per_sample); ++i)
      {
        // int16 output_sound = ((number_of_periods % wave_period) > (wave_period / 2) ? -3000 : 3000);
        float t = 2.0f * PI * number_of_periods / (float) wave_period;
        int16 output_sound = (int16) (sinf(t) * 3000);
        *sound_buffer_pointer++ = output_sound;
        *sound_buffer_pointer++ = output_sound;
        number_of_periods++;
      }
      SDL_QueueAudio(1, sound_buffer, bytes_to_write);
      free(sound_buffer);
    }

    if (!sound_playing)
    {
      SDL_PauseAudio(0);
      sound_playing = true;
    }

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
    int counters_per_frame = end_counter - last_counter;
    float counters_per_second = (((float) counters_per_frame * 1000.0f) / (float) performance_frequency);
    // printf("ms per frame: %f\n", counters_per_second);
    last_counter = end_counter;
  }

  free(backbuffer.memory);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
