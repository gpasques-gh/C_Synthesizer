#ifndef SYNTH_H
#define SYNTH_H

#include "defs.h"

typedef enum 
{
    ENV_IDLE,
    ENV_ATTACK,
    ENV_DECAY,
    ENV_SUSTAIN,
    ENV_RELEASE
} env_state_t;

typedef struct 
{
    double *attack, *decay, *sustain, *release;
    double output;
    env_state_t state;
} adsr_t;

typedef struct 
{
    double freq, phase;
    short wave;
} osc_t;

typedef struct 
{
    float alpha, prev, cutoff;
} lp_filter_t;

typedef struct 
{
    osc_t *oscillators;
    adsr_t *adsr;
    int active;
    int note;
    double velocity_amp;
} voice_t;

typedef struct 
{
    voice_t *voices;
    lp_filter_t *filter;
    double detune;
    double amp;
} synth_t;

// HELPERS
void change_freq(voice_t *voice, int note, int velocity, double detune);
const char *get_wave_name(int wave);
voice_t *get_free_voice(synth_t *synth);

// WAVEFORM RENDERING
void render_synth(synth_t *synth, short *buffer);

// SOUND PROCESSING
double adsr_process(adsr_t *adsr);
void lp_init(lp_filter_t *filter, float cutoff);
short lp_process(lp_filter_t *filter, short input);


#endif