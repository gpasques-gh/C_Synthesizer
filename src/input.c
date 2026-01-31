#include <SDL2/SDL.h>
#include "synth.h"
#include "defs.h"

int handle_input(note_t *note, SDL_Event event) {

    SDL_Keycode key = event.key.keysym.sym;

    switch(key) {
        case SDLK_UP:
            note->n_octave++;
            return 0;
        case SDLK_DOWN:
            note->n_octave--;
            return 0;
        case kC:
            change_note(note, nC, note->n_octave, note->n_duration);
            break;
        case kC_SHARP:
            change_note(note, nC_SHARP, note->n_octave, note->n_duration);
            break;
        case kD:
            change_note(note, nD, note->n_octave, note->n_duration);
            break;
        case kD_SHARP:
            change_note(note, nD_SHARP, note->n_octave, note->n_duration);
            break;
        case kE:
            change_note(note, nE, note->n_octave, note->n_duration);
            break;
        case kF:
            change_note(note, nF, note->n_octave, note->n_duration);
            break;
        case kF_SHARP:
            change_note(note, nF_SHARP, note->n_octave, note->n_duration);
            break;
        case kG:
            change_note(note, nG, note->n_octave, note->n_duration);
            break;
        case kG_SHARP:
            change_note(note, nG_SHARP, note->n_octave, note->n_duration);
            break;
        case kA:
            change_note(note, nA, note->n_octave, note->n_duration);
            break;
        case kA_SHARP:
            change_note(note, nA_SHARP, note->n_octave, note->n_duration);
            break;
        case kB:
            change_note(note, nB, note->n_octave, note->n_duration);
            break;
        default:
            return 0;
    }

    return 1;


}