#include <SDL2/SDL.h>

#include "defs.h"
#include "synth.h"
#include "keyboard.h"

/*
 * Get the keyboard input from the SDL key event and the keyboard layout (QWERTY or AZERTY)
 * Change the synth_t voices frequencies with the assigned note
 * Change the ADSR parameters when the assigned keys are being pressed
 * Change the cutoff, detune and amplification when the assigned keys are being pressed
 * Change the keyboard octave when UP or DOWN keys are being pressed
 */
void handle_input(synth_t *synth, int layout, int *octave)
{
    int octave_length = *octave * 12;
    if (IsKeyDown(kC_QWERTY))
        if (layout == QWERTY)
            assign_note(synth, octave_length + nC);
    if (IsKeyDown(kC_AZERTY))
        if (layout == AZERTY)
            assign_note(synth, octave_length + nC);
    if (IsKeyDown(kC_SHARP_QWERTY))
        if (layout == QWERTY)
            assign_note(synth, octave_length + nC_SHARP);
    if (IsKeyDown(kC_SHARP_AZERTY))
        if (layout == AZERTY)
            assign_note(synth, octave_length + nC_SHARP);
    if (IsKeyDown(kD))
        assign_note(synth, octave_length + nD);
    if (IsKeyDown(kD_SHARP))
        assign_note(synth, octave_length + nD_SHARP);
    if (IsKeyDown(kE))
        assign_note(synth, octave_length + nE);
    if (IsKeyDown(kF))
        assign_note(synth, octave_length + nF);
    if (IsKeyDown(kF_SHARP))
        assign_note(synth, octave_length + nF_SHARP);
    if (IsKeyDown(kG))
        assign_note(synth, octave_length + nG);
    if (IsKeyDown(kG_SHARP))
        assign_note(synth, octave_length + nG_SHARP);
    if (IsKeyDown(kA))
        assign_note(synth, octave_length + nA);
    if (IsKeyDown(kA_SHARP))
        assign_note(synth, octave_length + nA_SHARP);
    if (IsKeyDown(kB))
        assign_note(synth, octave_length + nB);
    if (IsKeyDown(KEY_UP))
        (*octave)++;
    else if (IsKeyDown(KEY_DOWN))
        (*octave)--;
}

/* Free the synth voices when their assigned note key are being released */
void handle_release(synth_t *synth, int layout, int octave)
{
    int octave_length = octave * 12;
    if (IsKeyReleased(kC_QWERTY))
        if (layout == QWERTY)
            release_note(synth, octave_length + nC);
    if (IsKeyReleased(kC_AZERTY))
        if (layout == AZERTY)
            release_note(synth, octave_length + nC);
    if (IsKeyReleased(kC_SHARP_QWERTY))
        if (layout == QWERTY)
            release_note(synth, octave_length + nC_SHARP);
    if (IsKeyReleased(kC_SHARP_AZERTY))
        if (layout == AZERTY)
            release_note(synth, octave_length + nC_SHARP);
    if (IsKeyReleased(kD))
        release_note(synth, octave_length + nD);
    if (IsKeyReleased(kD_SHARP))
        release_note(synth, octave_length + nD_SHARP);
    if (IsKeyReleased(kE))
        release_note(synth, octave_length + nE);
    if (IsKeyReleased(kF))
        release_note(synth, octave_length + nF);
    if (IsKeyReleased(kF_SHARP))
        release_note(synth, octave_length + nF_SHARP);
    if (IsKeyReleased(kG))
        release_note(synth, octave_length + nG);
    if (IsKeyReleased(kG_SHARP))
        release_note(synth, octave_length + nG_SHARP);
    if (IsKeyReleased(kA))
        release_note(synth, octave_length + nA);
    if (IsKeyReleased(kA_SHARP))
        release_note(synth, octave_length + nA_SHARP);
    if (IsKeyReleased(kB))
        release_note(synth, octave_length + nB);    
}

void assign_note(synth_t *synth, int midi_note)
{
    if (midi_note != -1)
    {
        int active_voices = 0;
        for (int v = 0; v < VOICES; v++) 
        {
            if (synth->voices[v].active && synth->voices[v].adsr->state != ENV_RELEASE)
                active_voices++;
            
            if (synth->voices[v].adsr->state == ENV_RELEASE)
            {
                synth->voices[v].active = 0;
                synth->voices[v].adsr->state = ENV_IDLE;
                synth->voices[v].adsr->output = 0.0;
            }
        }

        voice_t *free_voice = get_free_voice(synth);
        if (free_voice == NULL)
            return;
        change_freq(free_voice, midi_note, 127, synth->detune);
        if (active_voices == 0)
            synth->filter->adsr->state = ENV_ATTACK;
        return;
    }
}

void release_note(synth_t *synth, int midi_note)
{
    for (int v = 0; v < VOICES; v++)
        if (synth->voices[v].note == midi_note && synth->voices[v].active)
        {
            synth->voices[v].adsr->state = ENV_RELEASE;
            break;
        }
}