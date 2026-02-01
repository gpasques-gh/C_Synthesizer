#include <SDL2/SDL.h>

#include "kb_input.h"
#include "defs.h"
#include "synth.h"

/**
 * Keyboard Input handler
 * Change the note and the current octave and then update the synth oscillators
 */
void handle_input(SDL_Event *event, note_t *note, synth_3osc_t *synth) {

    SDL_Keycode key = event->key.keysym.sym;

    switch(key) {
        case kC:
            note->semitone = nC;
            change_osc_freq(synth, *note);
            break;
        case kC_SHARP:
            note->semitone = nC_SHARP;
            change_osc_freq(synth, *note);
            break;
        case kD:
            note->semitone = nD;
            change_osc_freq(synth, *note);
            break;
        case kD_SHARP:
            note->semitone = nD_SHARP;
            change_osc_freq(synth, *note);
            break;
        case kE:
            note->semitone = nE;
            change_osc_freq(synth, *note);
            break;
        case kF:
            note->semitone = nF;
            change_osc_freq(synth, *note);
            break;
        case kF_SHARP:
            note->semitone = nF_SHARP;
            change_osc_freq(synth, *note);
            break;
        case kG:
            note->semitone = nG;
            change_osc_freq(synth, *note);
            break;
        case kG_SHARP:
            note->semitone = nG_SHARP;
            change_osc_freq(synth, *note);
            break;
        case kA:
            note->semitone = nA;
            change_osc_freq(synth, *note);
            break;
        case kA_SHARP:
            note->semitone = nA_SHARP;
            change_osc_freq(synth, *note);
            break;
        case kB:
            note->semitone = nB;
            change_osc_freq(synth, *note);
            break;
        case SDLK_UP:
            note->octave++;
            break;
        case SDLK_DOWN:
            note->octave--;
            break;
        default:
            break;
        }

}