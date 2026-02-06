#ifndef INTERFACE_H
#define INTERFACE_H


#include <raygui.h>

#include "defs.h"
#include "synth.h"

typedef struct 
{
    float *attack, *decay, *sustain, *release;
    adsr_t *filter_envelope;
    synth_t *synth;
    int *dropbox_a, *dropbox_b, *dropbox_c; 
    bool *dropbox_a_b, *dropbox_b_b, *dropbox_c_b;
} params_t;

void render_informations(params_t *params);

void render_waveform(short *buffer);


/* Outputs a given MIDI note rectangle parameters (x, y, width and height) */
void get_key_position(int midi_note, int *x, int *y,
                      int *width, int *height, int *is_black);

/* Returns if a MIDI note is a assigned to a black key or not */
int is_black_key(int midi_note);


#endif