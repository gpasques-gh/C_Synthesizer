#ifndef SYNTH_H
#define SYNTH_H

typedef struct {
    short semitone;
    short octave;
    short duration;
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
    int frames_left;
    int frames_total;
    int active;
    short wave;
} osc_t;

typedef struct {
    osc_t *osc_a;
    osc_t *osc_b;
    osc_t *osc_c;
    adsr_t *adsr;
    int frames_left;
    int frames_total;
    int active;
} synth_3osc_t;

// SOUND RELATED
double get_adsr_envelope(synth_3osc_t *synth);
void change_osc_freq(synth_3osc_t *synth, note_t note);
char *get_wave_name(int wave);

// SOUND WAVES
void render_sound(osc_t *sound,  short *buffer);
void render_sine(osc_t *sound, short *buffer);
void render_square(osc_t *sound, short *buffer);
void render_triangle(osc_t *sound, short *buffer);
void render_sawtooth(osc_t *sound, short *buffer);
void render_synth3osc(synth_3osc_t *synth, short *mix_buffer);

#endif