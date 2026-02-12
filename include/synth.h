#ifndef SYNTH_H
#define SYNTH_H

#include <stdbool.h>

/* ADSR envelope states */
typedef enum
{
    ENV_IDLE,
    ENV_ATTACK,
    ENV_DECAY,
    ENV_SUSTAIN,
    ENV_RELEASE
} env_state_t;

/*
 * Time-based ADSR envelope structure
 * Each parameter is expressed in seconds
 * The sustain parameter controls the sound level
 * The output is the amplification coefficient of the envelope
 */
typedef struct
{
    float *attack, *decay, *sustain, *release;
    float output;
    env_state_t state;
} adsr_t;

/*
 * Oscillator structure
 * Wave can either be a sine, square, triangle or sawtooth
 */
typedef struct
{
    float freq, phase;
    int *wave;
} osc_t;

typedef struct 
{
    osc_t *osc;
    int mod_param;
} lfo_t;

/* Low-pass filter structure */
typedef struct
{
    float prev_input, prev_output, cutoff, env_cutoff, lfo_cutoff;
    adsr_t *adsr;
    bool env;
} lp_filter_t;

/*
 * 3 oscillators voice structure
 * Each voice has its own ADSR envelope and MIDI velocity amplification
 * The note is in MIDI range (0 to 127)
 */
typedef struct
{
    osc_t *oscillators;
    adsr_t *adsr;
    int active;
    int note;
    double velocity_amp;
} voice_t;

/*
 * Polyphonic synthesizer structure
 * Voices is an array of voice_t
 * Detune is between 0.0 and 1.0
 * Amplification is between 0.0 and 1.0
 */
typedef struct
{
    voice_t *voices;
    lp_filter_t *filter;
    lfo_t *lfo;
    float detune;
    float lfo_detune;
    float amp;
    float lfo_amp;
    int active_arp;
    float bpm;
    float active_arp_float;
    bool arp;
} synth_t;

/*
 * Process a sample from the ADSR envelope
 * Returns the envelope amplification coeficient
 */
float adsr_process(adsr_t *adsr);

/* Renders the synth_t voices into the temporary sound buffer */
void render_synth(synth_t *synth, short *buffer);

/*
 * Change the frequency of a voice_t oscillators with the given MIDI note and velocity
 * Multiplied by the synth_t detune coefficient
 */
void change_freq(voice_t *voice, int note,
                 int velocity, double detune);

/* Apply the detune change to the voices oscillators */
void apply_detune_change(synth_t *synth);

/* Get the literal name of a given waveform */
const char *get_wave_name(int wave);

/*
 * Process a sample with the low-pass filter
 * Returns the processed sample
 */
double
lp_process(lp_filter_t *filter, double input,
           float cutoff);

/*
 * Returns the first free voice from the synth_t
 * Used to assign a note send by MIDI or keyboard to the first free voice
 */
voice_t *get_free_voice(synth_t *synth);

/* Insertion sort algorithm for the voices of a synth_t, used for arpeggiator */
void sort_synth_voices(synth_t *synth);

#endif