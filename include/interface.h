#ifndef INTERFACE_H
#define INTERFACE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "synth.h"

typedef struct {
    SDL_Texture *title_texture;
    SDL_Texture *envelope_texture;
    SDL_Texture *waveform_texture;
    char last_title_text[256];
    char last_envelope_text[256];
    char last_waveform_text[256];
} text_cache_t;

void render_infos(synth_t synth, TTF_Font *font, SDL_Renderer *renderer, 
    double attack, double decay, double sustain, double release);
void cleanup_text_cache();
void render_waveform(SDL_Renderer *renderer, short *buffer);
void render_keyboard_base(SDL_Renderer *renderer);
void render_key(SDL_Renderer *renderer, int midi_note);
void get_key_position(int midi_note, int *x, int *y, 
    int *width, int *height, int *is_black);
#endif