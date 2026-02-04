#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "interface.h"
#include "synth.h"


void render_infos(synth_t synth, 
    TTF_Font *font, SDL_Renderer *renderer,
    double attack, double decay, double sustain, double release)
{
    SDL_Color black = {0, 0, 0, 255};
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Voice A : %d | Voice B : %d | Voice C : %d | Voice D : %d | Voice E : %d | Voice F : %d", 
    synth.voices[0].note, synth.voices[1].note,synth.voices[2].note, synth.voices[3].note, synth.voices[4].note, synth.voices[5].note);
    SDL_Surface *surface = TTF_RenderText_Solid(font, buffer, black);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect surface_rect = {
        .h = 50,
        .w = WIDTH,
        .x = 0,
        .y = 0
    }; 
    SDL_RenderCopy(renderer, texture, NULL, &surface_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    char buffer2[256];

    snprintf(buffer2, sizeof(buffer2), "Envelope - Attack: %.2f | Decay: %.2f | Sustain: %.2f | Release: %.2f", 
        attack, decay, sustain, release);
    SDL_Surface *surface2 = TTF_RenderText_Solid(font, buffer2, black);
    SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_Rect surface_rect2 = {
        .h = 50,
        .w = WIDTH,
        .x = 0,
        .y = 60
    };
    SDL_RenderCopy(renderer, texture2, NULL, &surface_rect2);
    SDL_FreeSurface(surface2);
    SDL_DestroyTexture(texture2);

    char buffer3[256];

    snprintf(buffer3, sizeof(buffer3), "Waveforms - Osc A: %s | Osc B: %s | Osc C: %s", 
        get_wave_name(synth.voices[0].oscillators[0].wave), get_wave_name(synth.voices[0].oscillators[1].wave), get_wave_name(synth.voices[0].oscillators[2].wave));
    SDL_Surface *surface3 = TTF_RenderText_Solid(font, buffer3, black);
    SDL_Texture *texture3 = SDL_CreateTextureFromSurface(renderer, surface3);
    SDL_Rect surface_rect3 = {
        .h = 50,
        .w = WIDTH,
        .x = 0,
        .y = 120
    };
    SDL_RenderCopy(renderer, texture3, NULL, &surface_rect3);
    SDL_FreeSurface(surface3);
    SDL_DestroyTexture(texture3);
}

void render_waveform(SDL_Renderer *renderer, short *buffer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    int mid_y = HEIGHT / 2;
    for (int i = 0; i < FRAMES - 1; i++) 
    {
        int x1 = (i * WIDTH) / FRAMES;
        int x2 = ((i + 1) * WIDTH) / FRAMES;

        int y1 = mid_y - ((buffer[i] * mid_y) / 32768);
        int y2 = mid_y - ((buffer[i + 1] * mid_y) / 32768);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void render_keyboard_base(SDL_Renderer *renderer)
{
    char black_keys_pattern[] = {1, 1, 0, 1, 1, 1, 0, 0};

    for (int i = 0; i < WHITE_KEYS; i++)
    {
        SDL_Rect key = 
        {
            .h = WHITE_KEYS_HEIGHT,
            .w = WHITE_KEYS_WIDTH,
            .x = i * WHITE_KEYS_WIDTH,
            .y = 0
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &key);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &key);
    }


    int white_key_index = 0;
    for (int octave = 0; octave < (WHITE_KEYS / 7); octave++)
    {
        for (int i = 0; i < 7; i++)
        {
            if (black_keys_pattern[i])
            {
                int x = (white_key_index * WHITE_KEYS_WIDTH) + WHITE_KEYS_WIDTH - (BLACK_KEYS_WIDTH / 2);
                SDL_Rect black_key = {
                    .h = BLACK_KEYS_HEIGHT,
                    .w = BLACK_KEYS_WIDTH,
                    .x = x,
                    .y = 0
                };
                SDL_RenderFillRect(renderer, &black_key);
            }
            white_key_index++;
            if (white_key_index >= WHITE_KEYS) break;
        }
        if (white_key_index >= WHITE_KEYS) break;
    }
}

void get_key_position(int midi_note, int *x, int *y, int *width, int *height, int *is_black)
{
    int note_in_octave = midi_note % 12;
    int octave = midi_note / 12;

    static const int black_keys[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    *is_black = black_keys[note_in_octave];

    static const int white_key_map[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
    int white_key_in_octave = white_key_map[note_in_octave];

    int white_key_index = (octave * 7) + white_key_in_octave;
    
    if (*is_black)
    {
        *width = WHITE_KEYS_WIDTH / 2;
        *height = (WHITE_KEYS_HEIGHT * 2) / 3;
        *x = (white_key_index * WHITE_KEYS_WIDTH) + WHITE_KEYS_WIDTH - (*width / 2);
        *y = HEIGHT - WHITE_KEYS_HEIGHT;
    }
    else
    {
        *width = WHITE_KEYS_WIDTH;
        *height = WHITE_KEYS_HEIGHT;
        *x = white_key_index * WHITE_KEYS_WIDTH;
        *y = HEIGHT - WHITE_KEYS_HEIGHT;
    }
}


void render_key(SDL_Renderer *renderer, int midi_note) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    int width = 0, height = 0, x = 0, y = 0, is_black = 0;
    get_key_position(midi_note, &x, &y, &width, &height, &is_black);

    SDL_Rect key =
    {
        .h = height,
        .w = width,
        .x = x,
        .y = y
    };

    SDL_RenderFillRect(renderer, &key);
}   