#ifndef SYNTH_H
#define SYNTH_H

typedef struct {
    short n_semitone;
    short n_octave;
    short n_duration;
} note_t;

typedef struct {
    double att;
    double dec;
    double sus;
    double rel;
} adsr_t;

typedef struct {
    double s_freq;
    double s_phase;
    int s_frames_left;
    int s_frames_total;
    int s_active;
    short s_wave;
    adsr_t *s_adsr;
} sound_t;

typedef struct {
    sound_t *osc_a;
    sound_t *osc_b;
    sound_t *osc_c;
} synth_3osc_t;

typedef struct {
    sound_t *osc_a;
    sound_t *osc_b;
} synth_2osc_t;

// SOUND RELATED
double get_adsr_envelope(sound_t *sound);
void change_note(note_t *note, short semitone, short octave, int duration);
void note_to_sound(note_t note, sound_t *sound);

// SOUND WAVES
void render_sound(sound_t *sound, short *buffer);
void render_sine(sound_t *sound, short *buffer);
void render_square(sound_t *sound, short *buffer);
void render_triangle(sound_t *sound, short *buffer);
void render_sawtooth(sound_t *sound, short *buffer);
void render_synth2osc(synth_2osc_t synth, short *mix_buffer);
void render_synth3osc(synth_3osc_t synth, short *mix_buffer);

#endif