#ifndef MIDI_H
#define MIDI_H

#include <alsa/asoundlib.h>
#include "synth.h"

int get_midi(snd_rawmidi_t *midi_in, synth_t *synth,
             double *attack, double *decay, double *sustain, double *release);

#endif