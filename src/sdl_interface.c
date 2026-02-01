#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "sdl_interface.h"
#include "synth.h"

void render_interface(note_t note, synth_3osc_t synth, TTF_Font *font, SDL_Renderer *renderer) {
    SDL_Color black = {0, 0, 0, 255};
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Semitone: %d | Octave: %d | Frequency: %.2f | Attack: %.2f | Decay: %.2f | Sustain: %.2f | Release: %.2f", 
    note.semitone, note.octave, synth.osc_a->freq, synth.adsr->att, synth.adsr->dec, synth.adsr->sus, synth.adsr->rel);
    SDL_Surface *surface = TTF_RenderText_Solid(font, buffer, black);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect surface_rect = {
        .h = 50,
        .w = 500,
        .x = 0,
        .y = 0
    };
    SDL_RenderCopy(renderer, texture, NULL, &surface_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);

    char buffer2[256];

    snprintf(buffer2, sizeof(buffer2), "Oscillator A wave: %s | Oscillator B wave: %s | Oscillator C wave: %s", 
        get_wave_name(synth.osc_a->wave), get_wave_name(synth.osc_b->wave), get_wave_name(synth.osc_c->wave));
    SDL_Surface *surface2 = TTF_RenderText_Solid(font, buffer2, black);
    SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
    SDL_Rect surface_rect2 = {
        .h = 50,
        .w = 500,
        .x = 0,
        .y = 60
    };
    SDL_RenderCopy(renderer, texture2, NULL, &surface_rect2);
    SDL_FreeSurface(surface2);
    SDL_DestroyTexture(texture2);
}
