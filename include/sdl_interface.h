#ifndef SDL_INTERFACE_H
#define SDL_INTERFACE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "synth.h"
#include "defs.h"

void render_interface(note_t note, synth_3osc_t synth, TTF_Font *font, SDL_Renderer *renderer);

#endif 