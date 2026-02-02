#ifndef SYNTH_H
#define SYNTH_H

#include "defs.h"

#define SINE_WAVE 0
#define SQUARE_WAVE 1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3

typedef struct {
    short semitone;
    short octave;
    short duration;
    short velocity;
} note_t;

typedef struct {
    double att;
    double dec;
    double sus;
    double rel;
} adsr_t;

typedef struct {
    double freq;
    double phase;
    short wave;
} osc_t;

typedef struct {
    float alpha; 
    float prev;
} lp_filter_t;

typedef struct {
    osc_t *osc_a;
    osc_t *osc_b;
    osc_t *osc_c;
    adsr_t *adsr;
    int frames_left;
    int frames_total;
    int active;
    int midi_note;
    double velocity_amplitude;
} synth_3osc_t;




typedef struct {
    synth_3osc_t *voice_a;
    synth_3osc_t *voice_b;
    synth_3osc_t *voice_c;
    synth_3osc_t *voice_d;
    synth_3osc_t *voice_e;
    synth_3osc_t *voice_f;
    lp_filter_t *lp_filter;
    double detune;
    int amplitude;
} poly_synth_t;


// SOUND RELATED
double get_adsr_envelope(synth_3osc_t *synth);
void change_osc_freq(synth_3osc_t *synth, note_t note, double detune);
char *get_wave_name(int wave);
synth_3osc_t *get_unused_voice(poly_synth_t *synth);

// SOUND WAVES
void render_osc(osc_t *osc,  short *buffer, int amplitude);
void render_sine(osc_t *osc, short *buffer, int amplitude);
void render_square(osc_t *osc, short *buffer, int amplitude);
void render_triangle(osc_t *osc, short *buffer, int amplitude);
void render_sawtooth(osc_t *osc, short *buffer, int amplitude);
void render_synth3osc(synth_3osc_t *synth, short *mix_buffer, int amplitude);
void render_poly_synth(poly_synth_t *synth, short *poly_buffer, int n_voices);

// FILTER
void lp_init(lp_filter_t *filter, float cutoff);
short lp_process(lp_filter_t *filter, short input);

#endif