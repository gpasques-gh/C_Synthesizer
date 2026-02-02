#include <alsa/asoundlib.h>

#include "synth.h"
#include "defs.h"
#include "midi.h"


/**
 * Function that read midi input from the snd_rawmidi_t input
 * Into a note_t struct that is then used to change the frequency of the synth_3osc_t oscillators
 */
int get_midi(snd_rawmidi_t *midi_in, note_t *note, poly_synth_t * synth, int *n_voices) {

    adsr_t *ptr_adsr = synth->voice_a->adsr;

    unsigned char midi_buffer[1024];
    ssize_t ret = snd_rawmidi_read(midi_in, midi_buffer, sizeof(midi_buffer));
    
    if (ret < 0) {
        return 1;
    }

    for (int i = 0; i + 2 < ret; i += 3) {
        unsigned char status = midi_buffer[i];
        unsigned char data1 = midi_buffer[i+1]; // Note if key pressed, CC value if knob turned
        unsigned char data2 = midi_buffer[i+2]; // Velocity if key pressed, Knob turn value if knob turned

        if ((status & PRESSED) == NOTE_ON && data2 > 0) {
            
            if (*n_voices < 6) {
                
                *n_voices += 1;
                fprintf(stderr, "voices on: %d\n", *n_voices);
                
            } else {
                return 0;
            }

            note->semitone = (data1 % 12);
            note->octave = (data1 / 12) - 1;
            note->velocity = data2;

            synth_3osc_t *unused_voice = get_unused_voice(synth);

            if (unused_voice == NULL) continue;

            change_osc_freq(unused_voice, *note, synth->detune);
            unused_voice->midi_note = data1;
        }

        if ((status & PRESSED) == NOTE_OFF || ((status & PRESSED) == NOTE_ON && data2 == 0)) {
            if (*n_voices >= 1) {
                *n_voices -= 1;

                if (synth->voice_a->midi_note == data1)
                    synth->voice_a->active = 0;
                if (synth->voice_b->midi_note == data1)
                    synth->voice_b->active = 0;
                if (synth->voice_c->midi_note == data1)
                    synth->voice_c->active = 0;
                if (synth->voice_d->midi_note == data1)
                    synth->voice_d->active = 0;
                if (synth->voice_e->midi_note == data1)
                    synth->voice_e->active = 0;
                if (synth->voice_f->midi_note == data1)
                    synth->voice_f->active = 0;

                fprintf(stderr, "voices off: %d\n", *n_voices);
                continue;
            }
        }

        if ((status & PRESSED) == KNOB_TURNED) {
            switch(data1) {
                case ARTURIA_ATT_KNOB:
                    ptr_adsr->att = (data2 / MIDI_MAX_VALUE);
                    break;
                case ARTURIA_DEC_KNOB:
                    ptr_adsr->dec = (data2 / MIDI_MAX_VALUE);
                    break;
                case ARTURIA_SUS_KNOB:
                    ptr_adsr->sus = (data2 / MIDI_MAX_VALUE);
                    break;
                case ARTURIA_REL_KNOB:
                    ptr_adsr->rel = (data2 / MIDI_MAX_VALUE);
                    break;
                case ARTURIA_OSC_A_KNOB:
                    synth->voice_a->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_a->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_a->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    break;
                case ARTURIA_OSC_B_KNOB:
                    synth->voice_b->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_b->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_b->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));;
                    break;
                case ARTURIA_OSC_C_KNOB:
                    synth->voice_c->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_c->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_c->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    break;
                case ARTURIA_OSC_D_KNOB:
                    synth->voice_d->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_d->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_d->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    break;
                case ARTURIA_OSC_E_KNOB:
                    synth->voice_e->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_e->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_e->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));;
                    break;
                case ARTURIA_OSC_F_KNOB:
                    synth->voice_f->osc_a->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_f->osc_b->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    synth->voice_f->osc_c->wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                    break;
                case ARTURIA_CUTOFF_KNOB:
                    double cutoff = (data2 / MIDI_MAX_VALUE) * (RATE / 2);
                    lp_init(synth->lp_filter, cutoff);
                    break;
                case ARTURIA_DETUNE_KNOB:
                    synth->detune = (data2 / MIDI_MAX_VALUE);
                    break;
                case ARTURIA_AMPLITUDE_KNOB:
                    synth->amplitude = (int)((data2 * MAX_AMPLITUDE)) / (MIDI_MAX_VALUE + 1);
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
    
}