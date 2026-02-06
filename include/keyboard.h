#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>
#include "synth.h"

/*
 * Get the keyboard input from the SDL key event and the keyboard layout (QWERTY or AZERTY)
 * Change the synth_t voices frequencies with the assigned note
 * Change the ADSR parameters when the assigned keys are being pressed
 * Change the cutoff, detune and amplification when the assigned keys are being pressed
 * Change the keyboard octave when UP or DOWN keys are being pressed
 */
void handle_input(synth_t *synth, int layout, int *octave);

/* Free the synth voices when their assigned note key are being released */
void handle_release(synth_t *synth, int layout, int octave);

/*
 * Converts a given key with its keyboard layout to a MIDI note
 * Returns -1 when the key is not assigned to a note, and the MIDI note otherwise
 */
void assign_note(synth_t *synth, int midi_note);
void release_note(synth_t *synth, int midi_note);

#endif