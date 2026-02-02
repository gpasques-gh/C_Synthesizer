#define _GNU_SOURCE
#include <math.h>
#include <string.h>
#include <stdio.h>
#include "synth.h"
#include "defs.h"

/**
 * Generate the amplitude for the current sound frame
 * based on the ADSR envelope of the given sound
 */
double get_adsr_envelope(synth_3osc_t *synth) {
    
    if (!synth->active) return 0.0;
    
    adsr_t *adsr = synth->adsr;

    int total_frames = synth->frames_total;
    int elapsed_frames = total_frames - synth->frames_left;

    int attack_frames = (int)(adsr->att * RATE);
    int decay_frames = (int)(adsr->dec * RATE);
    int release_frames = (int)(adsr->rel * RATE);

    if (attack_frames  < 1) attack_frames  = 1;
    if (decay_frames   < 1) decay_frames   = 1;
    if (release_frames < 1) release_frames = 1;
    
    int release_start = total_frames - release_frames;
    if (release_start < 0) release_start = 0;

    double amplitude = 0.0;
    
    // Attack
    if (elapsed_frames < attack_frames) {
        amplitude = (double) elapsed_frames / attack_frames;
    }
    // Decay
    else if (elapsed_frames < attack_frames + decay_frames) {
        int decay_pos = elapsed_frames - attack_frames;
        double decay_progress = (double) decay_pos / decay_frames;
        amplitude = 1.0 - decay_progress * (1.0 - adsr->sus);
    }

    // Sustain
    else if (elapsed_frames < release_start) {
        amplitude = adsr->sus;
    }
    // Release
    else {
        int release_pos = elapsed_frames - release_start;
        double release_progress = (double) release_pos / release_frames;
        amplitude = adsr->sus * (1.0 - release_progress);
        if (amplitude < 0.0) amplitude = 0.0;
    }

    return amplitude;
}

/**
 * Generate a sine wave with the given sound structure into the sound buffer
 * The buffer is then read into the snd_pcm 
 */
void render_sine(osc_t *osc, short *buffer, int amplitude) {

    double phase_inc = 2.0 * M_PI * osc->freq / RATE;

    for (int i = 0; i < FRAMES; i++) {
        
        double sample = sin(osc->phase);
        buffer[i] = (short)(amplitude * sample);
        
        osc->phase += phase_inc;
        if (osc->phase >= 2 * M_PI) osc->phase -= 2 * M_PI;
    }
}


/**
 * Generate a square wave with the given sound structure into the sound buffer
 * The buffer is then read into the snd_pcm 
 */
void render_square(osc_t *osc, short *buffer, int amplitude) {

    double phase_inc = osc->freq / RATE;

    for (int i = 0; i < FRAMES; i++) {

        double sample = (osc->phase < 0.5) ? 1.0 : -1.0;

        buffer[i] = (short)(amplitude * sample);

        osc->phase += phase_inc;

        if (osc->phase >= 1.0) osc->phase -= 1.0;
    }
}

/**
 * Generate a triangle wave with the given sound structure into the sound buffer
 * The buffer is then read into the snd_pcm 
 */
void render_triangle(osc_t *osc, short *buffer, int amplitude) {

    double phase_inc = osc->freq / RATE;

    for (int i = 0; i < FRAMES; i++) {

    
        double sample = 1.0 - 4.0 * fabs(osc->phase - 0.5);
        buffer[i] = (short)(amplitude * sample);

        osc->phase += phase_inc;
        if (osc->phase >= 1.0) osc->phase -= 1.0;
    }

}

/**
 * Generate a sawtooth wave with the given sound structure into the sound buffer
 * The buffer is then read into the snd_pcm 
 */
void render_sawtooth(osc_t *osc, short *buffer, int amplitude) {

    double phase_inc = osc->freq / RATE;

    for (int i = 0; i < FRAMES; i++) {
        double sample = 2.0 * osc->phase - 1.0;
        buffer[i] = (short)(amplitude * sample);

        osc->phase += phase_inc;
        if (osc->phase >= 1.0) osc->phase -= 1.0;
    }
}

/**
 * Generates the sound frames into the frame buffer
 * The buffer is then read by the snd_pcm into the sound card
 */
void render_osc(osc_t *osc, short *buffer, int amplitude) {

    switch(osc->wave) {
        case SINE_WAVE:
            render_sine(osc, buffer, amplitude);
            break;
        case SQUARE_WAVE:
            render_square(osc, buffer, amplitude);
            break;
        case TRIANGLE_WAVE:
            render_triangle(osc, buffer, amplitude);
            break;
        case SAWTOOTH_WAVE:
            render_sawtooth(osc, buffer, amplitude);
            break;
        default:
            break;
    }
}

/**
 * Generate the sound of the three oscillators of a 3 OSC synth
 * Into the mixed sound frame buffer
 */
void render_synth3osc(synth_3osc_t *synth, short *mix_buffer, int amplitude) {

    if (!synth->active || synth->frames_left == 0) {
        for (int i = 0; i < FRAMES; i++) {
            mix_buffer[i] = 0;
        }
        return;
    }

    short buffer_osc_a[FRAMES];
    short buffer_osc_b[FRAMES];
    short buffer_osc_c[FRAMES];
    render_osc(synth->osc_a, buffer_osc_a, amplitude);
    render_osc(synth->osc_b, buffer_osc_b, amplitude);
    render_osc(synth->osc_c, buffer_osc_c, amplitude);

    double envelope = get_adsr_envelope(synth);

    for (int i = 0; i < FRAMES; i++) {
        int mixed = buffer_osc_a[i] + buffer_osc_b[i] + buffer_osc_c[i];
        mixed /= 3;
        if (mixed > 32767) mixed = 32767;
        if (mixed < -32768) mixed = -32768;
        mix_buffer[i] = mixed * envelope;
        
        int attack_frames = (int)(synth->adsr->att * RATE);
        int decay_frames = (int)(synth->adsr->dec * RATE);

        if (synth->frames_left > 0) synth->frames_left--;
        else synth->frames_left = synth->frames_total - (attack_frames + decay_frames);
    }
}

void render_poly_synth(poly_synth_t *synth, short *poly_buffer, int n_voices) {

    short buffer_voice_a[FRAMES];
    short buffer_voice_b[FRAMES];
    short buffer_voice_c[FRAMES];
    short buffer_voice_d[FRAMES];
    short buffer_voice_e[FRAMES];
    short buffer_voice_f[FRAMES];
    
    //fprintf(stderr, "voices render: %d", n_voices);

    if (n_voices == 0) {
        synth->voice_a->active = 0;
        synth->voice_b->active = 0;
        synth->voice_c->active = 0;
        synth->voice_d->active = 0;
        synth->voice_e->active = 0;
        synth->voice_f->active = 0;
        for (int i = 0; i < FRAMES; i++) {
            poly_buffer[i] = 0;
        }
        return;
    }

    render_synth3osc(synth->voice_a, buffer_voice_a, synth->amplitude);
    render_synth3osc(synth->voice_b, buffer_voice_b, synth->amplitude);
    render_synth3osc(synth->voice_c, buffer_voice_c, synth->amplitude);
    render_synth3osc(synth->voice_d, buffer_voice_d, synth->amplitude);
    render_synth3osc(synth->voice_e, buffer_voice_e, synth->amplitude);
    render_synth3osc(synth->voice_f, buffer_voice_f, synth->amplitude);
   


    for (int i = 0; i < FRAMES; i++) {
        int poly_mix = buffer_voice_a[i] + buffer_voice_b[i] + buffer_voice_c[i] + buffer_voice_d[i] + buffer_voice_e[i] + buffer_voice_f[i];
        poly_mix /= 6;
        if (poly_mix > 32767) poly_mix = 32767;
        if (poly_mix < -32768) poly_mix = -32768;
        poly_buffer[i] = lp_process(synth->lp_filter, poly_mix);
    } 
}



/**
 * Converts a note structure to a sound structure
 * Generate the frequency of the given note (semitone + octave)
 * And set the sound frames to the default
 */
void change_osc_freq(synth_3osc_t *synth, note_t note, double detune) {
    // Getting the difference between the given note and A4
    double a4_diff = ((note.octave * 12) + note.semitone) - A4_POSITION;

    synth->frames_total = (note.duration * 1000) * RATE / 1000;
    synth->frames_left = synth->frames_total;
    synth->active = 1;
    synth->velocity_amplitude = (note.velocity / 127.0);

    synth->osc_a->freq= A_4 * pow(2, a4_diff / 12);
    synth->osc_a->phase = 0.0;  
    
    synth->osc_b->freq= A_4 * pow(2, a4_diff / 12) + (detune * 5);
    synth->osc_b->phase = 0.0;

    synth->osc_c->freq= A_4 * pow(2, a4_diff / 12) - (detune * 5);
    synth->osc_c->phase = 0.0;
}

/**
 * Changes the values of a given note structure
 */
void change_note(note_t *note, short semitone, short octave, int duration) {
    note->semitone = semitone;
    note->octave = octave;
    note->duration = duration;
}

/**
 * Helper function that give the name a given wave
 * Used in sdl_interface.c
 */
char *get_wave_name(int wave) {
    switch(wave) {
        case SINE_WAVE:
            return "Sine wave";
        case SQUARE_WAVE:
            return "Square wave";
        case TRIANGLE_WAVE:
            return "Triangle wave";
        case SAWTOOTH_WAVE:
            return "Sawtooth wave";
        default:
            return "Unknown wave";
    }
}

/**
 * Init function for a low-pass filter with the given cutoff
 */
void lp_init(lp_filter_t *filter, float cutoff) {
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float dt = 1.0f / RATE;
    filter->alpha = dt / (rc + dt);
    filter->prev = 0.0f;
}

/**
 * Function that processes a sound frame with the given low-pass filter
 */
short lp_process(lp_filter_t *filter, short input) {
    float x = (float)input;
    filter->prev = filter->prev + filter->alpha * (x - filter->prev);
    return (short)filter->prev;
}

synth_3osc_t *get_unused_voice(poly_synth_t *synth) {
    synth_3osc_t *voices[6] = {
        synth->voice_a,
        synth->voice_b,
        synth->voice_c,
        synth->voice_d,
        synth->voice_e,
        synth->voice_f
    };

    for (int i = 0; i < 6; i++) {
        if (!voices[i]->active) {
            return voices[i];
        }
    }

    return NULL;
}

