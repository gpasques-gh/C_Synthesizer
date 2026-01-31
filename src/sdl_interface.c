#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "sdl_interface.h"
#include "synth.h"

void render_interface(note_t note, synth_3osc_t synth, TTF_Font *font, SDL_Renderer *renderer) {
    SDL_Color black = {0, 0, 0, 255};
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Semitone: %d | Octave: %d | Frequency: %.2f", 
    note.n_semitone, note.n_octave, synth.osc_a->s_freq);
    SDL_Surface *surface = TTF_RenderText_Solid(font, buffer, black);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect surface_rect = {
        .h = 20,
        .w = 200,
        .x = 0,
        .y = 0
    };
    SDL_RenderCopy(renderer, texture, NULL, &surface_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
