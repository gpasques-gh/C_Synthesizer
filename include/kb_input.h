#ifndef KEYBOARD_INPUT_H
#define KEYBOARD_INPUT_H

#include <SDL2/SDL.h>
#include "defs.h"
#include "synth.h"

void handle_input(SDL_Event *event, note_t *note, synth_3osc_t *synth);

#endif