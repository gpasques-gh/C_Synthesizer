#ifndef INPUT_H
#define INPUT_H

#include <SDL2/SDL.h>

#include "defs.h"
#include "synth.h"

int handle_input(note_t *note, SDL_Event event);

#endif