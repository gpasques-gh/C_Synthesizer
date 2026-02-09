#include <alsa/asoundlib.h>

#include "defs.h"
#include "synth.h"
#include "midi.h"

/*
 * Get the MIDI input from the ALSA RawMIDI input (snd_rawmidi_t)
 * Activate the synth voices and update their frequencies with the given note
 * Turn off the synth voices when their assigned note are being released
 * Change the ADSR parameters when the assigned knobs are being triggered
 * Change the cutoff, detune and amplification when the assigned knobs are being triggered
 */
int get_midi(snd_rawmidi_t *midi_in, synth_t *synth,
             float *attack, float *decay, float *sustain, float *release)
{   
    /* Reading the MIDI stream */
    unsigned char midi_buffer[1024];
    ssize_t ret = snd_rawmidi_read(midi_in, midi_buffer, sizeof(midi_buffer));

    if (ret < 0)
        return 1;

    for (int i = 0; i + 2 < ret; i += 3)
    {   /* Getting the MIDI bytes informations */
        unsigned char status = midi_buffer[i];
        unsigned char data1 = midi_buffer[i + 1];
        unsigned char data2 = midi_buffer[i + 2];

        /* Note on is sent by the MIDI device */
        if ((status & PRESSED) == NOTE_ON && data2 > 0)
        {
            int active_voices = 0;

            for (int v = 0; v < VOICES; v++)
            {   
                if (synth->voices[v].active && synth->voices[v].adsr->state != ENV_RELEASE)
                    active_voices++;

                /* Cutting released voices to avoid some voices getting blocked */
                if (synth->voices[v].adsr->state == ENV_RELEASE)
                {
                    synth->voices[v].active = 0;
                    synth->voices[v].adsr->state = ENV_IDLE;
                    synth->voices[v].adsr->output = 0.0;
                }
            }

            /* Getting the first free voice from the synth */
            voice_t *free_voice = get_free_voice(synth);
            if (free_voice == NULL)
                continue;
            /* Changing the frequency of the voice oscillators */
            change_freq(free_voice, data1, data2, synth->detune);
            if (active_voices == 0)
                /* If no voices are active or in release state, start the filter envelope */
                synth->filter->adsr->state = ENV_ATTACK;
        }
        /* Note off is sent by the MIDI device */
        else if ((status & PRESSED) == NOTE_OFF ||
                 ((status & PRESSED) == NOTE_ON && data2 == 0))
            for (int v = 0; v < VOICES; v++)
            {   /* We cut the voice that has the sent MIDI note assigned */
                if (synth->voices[v].note == data1 && synth->voices[v].active)
                {
                    synth->voices[v].adsr->state = ENV_RELEASE;
                    break; /* Multiple voices can't share the same note, so we break */
                }
            }
        /* If a knob is turned on the MIDI device */
        else if ((status & PRESSED) == KNOB_TURNED)
        {
            /* Changing the synth parameters
             based on which knob is turned */
            switch (data1)
            {
            case ARTURIA_ATT_KNOB:
                *attack = ((float)data2 / MIDI_MAX_VALUE) * 2.0;
                break;
            case ARTURIA_DEC_KNOB:
                *decay = ((float)data2 / MIDI_MAX_VALUE) * 2.0;
                break;
            case ARTURIA_SUS_KNOB:
                *sustain = (float)data2 / MIDI_MAX_VALUE;
                break;
            case ARTURIA_REL_KNOB:
                *release = ((float)data2 / MIDI_MAX_VALUE) * 1.0;
                break;
            case ARTURIA_CUTOFF_KNOB:
                synth->filter->cutoff = ((float)data2 / MIDI_MAX_VALUE);
                break;
            case ARTURIA_DETUNE_KNOB:
                synth->detune = ((float)data2 / MIDI_MAX_VALUE);
                apply_detune_change(synth);
                break;
            case ARTURIA_AMPLITUDE_KNOB:
                synth->amp = ((float)data2 / MIDI_MAX_VALUE) * 1.0;
                break;
            default:
                break;
            }
        }
    }
    return 0;
}