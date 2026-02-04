#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>
#include "synth.h"

void handle_input(SDL_Keycode key, synth_t *synth, SDL_Renderer *renderer, int layout, int *octave,
                  double *attack, double *decay, double *sustain, double *release);
void handle_release(SDL_Keycode key, synth_t *synth, SDL_Renderer *renderer, int layout, int octave);
int key_to_note(SDL_Keycode key, int kb_layout, int octave);
#endif