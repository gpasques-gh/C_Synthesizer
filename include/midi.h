#ifndef MIDI_H
#define MIDI_H

#include <alsa/asoundlib.h>
#include "synth.h"

int get_midi(snd_rawmidi_t *midi_in, note_t *pnote) {
    
    if (midi_in) {
        unsigned char buffer[1024];
        ssize_t ret;
        while(1) {
            ret = snd_rawmidi_read(midi_in, buffer, sizeof(buffer));
            if (ret == -EAGAIN) {
                continue;
            }
            
            if (ret < 0) {
                break;
            } else if (ret > 0) {
                for (int i = 0; i + 2< ret; i+=3) {
                    unsigned char status = buffer[i];
                    unsigned char note = buffer[i + 1];
                    unsigned char vel = buffer[i + 2];

                    if ((status & 0xF0) == 0x90 && vel > 0) {
                        pnote->n_semitone = (note % 12) + 1;
                        pnote->n_octave = (note / 12) - 1;
                
                        return 1;
                    }
                }
            }
        }
    }
    
    return 0;
}

#endif