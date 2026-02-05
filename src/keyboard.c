#include <SDL2/SDL.h>

#include "defs.h"
#include "synth.h"
#include "keyboard.h"
#include "interface.h"

/*
 * Get the keyboard input from the SDL key event and the keyboard layout (QWERTY or AZERTY)
 * Change the synth_t voices frequencies with the assigned note
 * Change the ADSR parameters when the assigned keys are being pressed
 * Change the cutoff, detune and amplification when the assigned keys are being pressed
 * Change the keyboard octave when UP or DOWN keys are being pressed
 */
void handle_input(SDL_Keycode key, synth_t *synth, int layout, int *octave,
                  double *attack, double *decay, double *sustain, double *release)
{
    int midi_note = key_to_note(key, layout, *octave);

    if (midi_note != -1)
    {
        for (int v = 0; v < VOICES; v++)
            if (synth->voices[v].adsr->state == ENV_RELEASE)
            {
                synth->voices[v].active = 0;
                synth->voices[v].adsr->state = ENV_IDLE;
                synth->voices[v].adsr->output = 0.0;
            }

        voice_t *free_voice = get_free_voice(synth);
        if (free_voice == NULL)
            return;
        change_freq(free_voice, midi_note, 127, synth->detune);
        return;
    }

    switch (key)
    {
    case ATTACK_INCREMENT_QWERTY:
        if (layout == QWERTY)
        {
            *attack += 0.05;
            if (*attack > 1.05)
                *attack = 0.0;
        }
        break;
    case ATTACK_INCREMENT_AZERTY:
        if (layout == AZERTY)
        {
            *attack += 0.05;
            if (*attack > 1.05)
                *attack = 0.0;
        }
        break;
    case DECAY_INCREMENT:
        *decay += 0.05;
        if (*decay > 1.05)
            *decay = 0.0;
        break;
    case SUSTAIN_INCREMENT:
        *sustain += 0.05;
        if (*sustain > 1.05)
            *sustain = 0.0;
        break;
    case RELEASE_INCREMENT:
        *release += 0.05;
        if (*release > 1.05)
            *release = 0.0;
        break;
    case OSC_A_WAVE_INCREMENT:
        for (int v = 0; v < VOICES; v++)
            synth->voices[v].oscillators[0].wave = (synth->voices[v].oscillators[0].wave + 1) % 4;
        break;
    case OSC_B_WAVE_INCREMENT:
        for (int v = 0; v < VOICES; v++)
            synth->voices[v].oscillators[1].wave = (synth->voices[v].oscillators[1].wave + 1) % 4;
        break;
    case OSC_C_WAVE_INCREMENT:
        for (int v = 0; v < VOICES; v++)
            synth->voices[v].oscillators[2].wave = (synth->voices[v].oscillators[2].wave + 1) % 4;
        break;
    case AMPLITUDE_INCREMENT:
        synth->amp += 0.05;
        if (synth->amp > 1.05)
            synth->amp = 0.0;
        break;
    case CUTOFF_INCREMENT:
        synth->filter->cutoff += 0.05;
        if (synth->filter->cutoff > 1.05)
            synth->filter->cutoff = 0.0;
        break;
    case DETUNE_INCREMENT:
        synth->detune += 0.05;
        if (synth->detune > 1.05)
            synth->detune = 0.0;
        break;
    case SDLK_UP:
        (*octave)++;
        for (int v = 0; v < VOICES; v++)
        {
            synth->voices[v].adsr->state = ENV_RELEASE;
        }  
        break;
    case SDLK_DOWN:
        (*octave)--;
        for (int v = 0; v < VOICES; v++)
        {
            synth->voices[v].adsr->state = ENV_RELEASE;
        }
        break;
    default:
        break;
    }
}

/* Free the synth voices when their assigned note key are being released */
void handle_release(SDL_Keycode key, synth_t *synth, int layout, int octave)
{
    int midi_note = key_to_note(key, layout, octave);

    for (int v = 0; v < VOICES; v++)
        if (synth->voices[v].note == midi_note && synth->voices[v].active)
        {
            synth->voices[v].adsr->state = ENV_RELEASE;
            break;
        }
}

/*
 * Converts a given key with its keyboard layout to a MIDI note
 * Returns -1 when the key is not assigned to a note, and the MIDI note otherwise
 */
int key_to_note(SDL_Keycode key, int kb_layout, int octave)
{
    int midi_note = -1;
    int octave_length = octave * 12;

    switch (key)
    {
    case kC_QWERTY:
        if (kb_layout == QWERTY)
            midi_note = octave_length + nC;
        break;
    case kC_AZERTY:
        if (kb_layout == AZERTY)
            midi_note = octave_length + nC;
        break;
    case kC_SHARP_QWERTY:
        if (kb_layout == QWERTY)
            midi_note = octave_length + nC_SHARP;
        break;
    case kC_SHARP_AZERTY:
        if (kb_layout == AZERTY)
            midi_note = octave_length + nC_SHARP;
        break;
    case kD:
        midi_note = octave_length + nD;
        break;
    case kD_SHARP:
        midi_note = octave_length + nD_SHARP;
        break;
    case kE:
        midi_note = octave_length + nE;
        break;
    case kF:
        midi_note = octave_length + nF;
        break;
    case kF_SHARP:
        midi_note = octave_length + nF_SHARP;
        break;
    case kG:
        midi_note = octave_length + nG;
        break;
    case kG_SHARP:
        midi_note = octave_length + nG_SHARP;
        break;
    case kA:
        midi_note = octave_length + nA;
        break;
    case kA_SHARP:
        midi_note = octave_length + nA_SHARP;
        break;
    case kB:
        midi_note = octave_length + nB;
        break;
    default:
        break;
    }
    return midi_note;
}