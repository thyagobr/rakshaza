#include <SDL.h>
#include <time.h>

Uint64 performance_frequency;
Uint64 last_counter;
Uint64 end_counter;
bool has_resized;
int colour_offset = 0;
SDL_Joystick *joystick;

#define PI 3.14159265358979f

struct Backbuffer {
  void *memory;
  int width;
  int height;
} backbuffer;

struct Soundbuffer {
  void* memory;
  // 32 bits for stereo sample: 16 for left, 16 for right
  // we need this out every second
  int sampling_rate;
  Sint16 tone_volume;
  int tone_frequency_hz;
  int number_of_periods;
  int wave_period;
  int bytes_per_sample;
  int max_bytes_to_write;
} sound_buffer;

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

void output_audio(Soundbuffer *sound_buffer, int bytes_to_write)
{
  Sint16* sound_buffer_pointer = (Sint16*) sound_buffer->memory;
  int samples_per_frame = (bytes_to_write / sound_buffer->bytes_per_sample);
  for (int i = 0; i < samples_per_frame; ++i)
  {
    // Sint16 output_sound = ((number_of_periods % wave_period) > (wave_period / 2) ? -3000 : 3000);
    float t = 2.0f * PI * sound_buffer->number_of_periods / (float) sound_buffer->wave_period;
    Sint16 output_sound = (Sint16) (sinf(t) * sound_buffer->tone_volume);
    *sound_buffer_pointer++ = output_sound;
    *sound_buffer_pointer++ = output_sound;
    sound_buffer->number_of_periods++;
  }
}

void render(Backbuffer *backbuffer)
{
  int pitch = backbuffer->width * 4;
  int max_width = backbuffer->width;
  int max_height = backbuffer->height;
  Uint8 *row = (Uint8*) backbuffer->memory;
  for (int h = 0; h < max_height; ++h)
  {
    Uint32* pixel = (Uint32*) (row + h * pitch);
    for (int w = 0; w < max_width; ++w)
    {
      *pixel++ = (0xFF000000|(colour_offset + w)<<16|h<<8);
      // *pixel++ = 0xFF912CEE + colour_offset;
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
  has_resized = false;
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

  sound_buffer.sampling_rate = 48000;
  sound_buffer.tone_volume = 3000;
  sound_buffer.tone_frequency_hz = 256;
  sound_buffer.number_of_periods = 0;
  sound_buffer.wave_period = sound_buffer.sampling_rate / sound_buffer.tone_frequency_hz;
  sound_buffer.bytes_per_sample = sizeof(short) * 2;
  sound_buffer.max_bytes_to_write = 48000 * sound_buffer.bytes_per_sample;

  audio_init(sound_buffer.sampling_rate, (sound_buffer.sampling_rate * sound_buffer.bytes_per_sample) / 30);
  bool sound_playing = false;

  // joystick
  if (SDL_NumJoysticks() > 0)
  {
    joystick = SDL_JoystickOpen(0);
    if (!joystick)
    {
      printf("This joystick is not suppoerted. Sorry.\n");
    }
  }


  while (game_running)
  {
    colour_offset++;
    last_counter = SDL_GetPerformanceCounter();

    if (!sound_playing)
    {
      SDL_PauseAudio(0);
      sound_playing = true;
    }

    if (has_resized)
    {
      free(backbuffer.memory);
      backbuffer.memory = malloc(backbuffer.width * backbuffer.height * 4); 
      texture = SDL_CreateTexture(renderer,
          SDL_PIXELFORMAT_ARGB8888,
          SDL_TEXTUREACCESS_STREAMING,
          backbuffer.width, backbuffer.height);
      has_resized = false;
    }
    render(&backbuffer);
    SDL_UpdateTexture(texture, NULL, backbuffer.memory, backbuffer.width * 4);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    // Joystick detection
    if (joystick) {
      Sint16 x_move = SDL_JoystickGetAxis(joystick, 0);
      if (x_move)
      {
        colour_offset += x_move % 10;
        last_counter = SDL_GetPerformanceCounter();
      }
    }

    int bytes_to_write = sound_buffer.max_bytes_to_write - SDL_GetQueuedAudioSize(1);
    if (bytes_to_write)
    {
      sound_buffer.memory = malloc(bytes_to_write);
      output_audio(&sound_buffer, bytes_to_write);
      SDL_QueueAudio(1, sound_buffer.memory, bytes_to_write);
      end_counter = SDL_GetPerformanceCounter();
      free(sound_buffer.memory);
    }


    while (SDL_PollEvent(&event))
    {
      switch(event.type)
      {
        case SDL_QUIT:
          {
            printf("Quitting.\n");
            game_running = false;
          } break;
        case SDL_WINDOWEVENT:
          {
            switch(event.window.event)
            {
              case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                  backbuffer.width = event.window.data1;
                  backbuffer.height = event.window.data2;
                  has_resized = true;
                } break;
            }
          } break;
      }
    }
    int counters_per_frame = end_counter - last_counter;
    float counters_per_second = (((float) counters_per_frame * 1000.0f) / (float) performance_frequency);
    if (counters_per_second > 0) {
      printf("ms per frame: %f\n", counters_per_second);
    }
    last_counter = end_counter;
  }

  free(backbuffer.memory);
  if (joystick) {
    SDL_JoystickClose(joystick);
  }
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
}
