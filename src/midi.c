#include <alsa/asoundlib.h>

#include "defs.h"
#include "synth.h"
#include "midi.h"

/**
 * Get the MIDI input from the ALSA RawMIDI input (snd_rawmidi_t)
 * Activate the synth voices and update their frequencies with the given note
 * Turn off the synth voices when their assigned note are being released
 * Change the ADSR parameters when the assigned knobs are being triggered
 * Change the cutoff, detune and amplification when the assigned knobs are being triggered
 */
int get_midi(snd_rawmidi_t *midi_in, synth_t *synth,
             double *attack, double *decay, double *sustain, double *release)
{
    unsigned char midi_buffer[1024];
    ssize_t ret = snd_rawmidi_read(midi_in, midi_buffer, sizeof(midi_buffer));

    if (ret < 0)
        return 1;

    for (int i = 0; i + 2 < ret; i += 3)
    {
        unsigned char status = midi_buffer[i];
        unsigned char data1 = midi_buffer[i + 1];
        unsigned char data2 = midi_buffer[i + 2];

        if ((status & PRESSED) == NOTE_ON && data2 > 0)
        {
            for (int v = 0; v < VOICES; v++)
            {
                if (synth->voices[v].adsr->state == ENV_RELEASE)
                {
                    synth->voices[v].active = 0;
                    synth->voices[v].adsr->state = ENV_IDLE;
                    synth->voices[v].adsr->output = 0.0;
                }
            }

            voice_t *free_voice = get_free_voice(synth);
            if (free_voice == NULL)
                continue;
            change_freq(free_voice, data1, data2, synth->detune);
        }
        else if ((status & PRESSED) == NOTE_OFF ||
                 ((status & PRESSED) == NOTE_ON && data2 == 0))
            for (int v = 0; v < VOICES; v++)
            {
                if (synth->voices[v].note == data1 && synth->voices[v].active)
                {
                    synth->voices[v].adsr->state = ENV_RELEASE;
                    break;
                }
            }
        else if ((status & PRESSED) == KNOB_TURNED)
        {
            switch (data1)
            {
            case ARTURIA_ATT_KNOB:
                *attack = ((double)data2 / MIDI_MAX_VALUE) * 2.0;
                break;
            case ARTURIA_DEC_KNOB:
                *decay = ((double)data2 / MIDI_MAX_VALUE) * 2.0;
                break;
            case ARTURIA_SUS_KNOB:
                *sustain = (double)data2 / MIDI_MAX_VALUE;
                break;
            case ARTURIA_REL_KNOB:
                *release = ((double)data2 / MIDI_MAX_VALUE) * 1.0;
                break;
            case ARTURIA_OSC_A_KNOB:
                for (int v = 0; v < VOICES; v++)
                    synth->voices[v].oscillators[0].wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                break;
            case ARTURIA_OSC_B_KNOB:
                for (int v = 0; v < VOICES; v++)
                    synth->voices[v].oscillators[1].wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                break;
            case ARTURIA_OSC_C_KNOB:
                for (int v = 0; v < VOICES; v++)
                    synth->voices[v].oscillators[2].wave = (int)((data2 * 4) / (MIDI_MAX_VALUE + 1));
                break;
            case ARTURIA_CUTOFF_KNOB:
            {
                synth->filter->cutoff = ((double)data2 / MIDI_MAX_VALUE) * (RATE / 2);
                lp_init(synth->filter, synth->filter->cutoff);
                break;
            }
            case ARTURIA_DETUNE_KNOB:
                synth->detune = ((double)data2 / MIDI_MAX_VALUE);
                apply_detune_change(synth);
                break;
            case ARTURIA_AMPLITUDE_KNOB:
                synth->amp = ((double)data2 / MIDI_MAX_VALUE) * 1.0;
                break;
            default:
                break;
            }
        }
    }

    return 0;
}