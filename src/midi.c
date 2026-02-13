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
    unsigned char midi_buffer[1024];
    ssize_t ret = snd_rawmidi_read(midi_in, midi_buffer, sizeof(midi_buffer));

    if (ret < 0)
    {
        fprintf(stderr, "midi read error\n");
        return 1;
    }
        

    for (int i = 0; i + 2 < ret; i += 3)
    {   /* Getting the MIDI bytes informations */
        unsigned char status = midi_buffer[i];
        unsigned char data1 = midi_buffer[i + 1];
        unsigned char data2 = midi_buffer[i + 2];

        if ((status & PRESSED) == NOTE_ON && data2 > 0)
        {
            int pressed_voices = 0;

            for (int v = 0; v < VOICES; v++)
            {   
                if (synth->voices[v].pressed)
                {
                    pressed_voices++;
                }
                
                if (synth->voices[v].adsr->state == ENV_RELEASE && !synth->arp)
                {
                    synth->voices[v].adsr->state = ENV_IDLE;
                }
            }

            voice_t *free_voice = get_free_voice(synth);
            if (free_voice == NULL)
            {
                continue;
            }   
            free_voice->pressed = 1;
            change_freq(free_voice, data1, data2, synth->detune);
            if (pressed_voices == 0 && synth->filter->env)
            {
                synth->filter->adsr->state = ENV_ATTACK;
            }
                

            if (synth->arp)
            {
                sort_synth_voices(synth);
                if (pressed_voices == 0)
                {
                    synth->active_arp_float = 1.0;
                }
            }
        }
        else if ((status & PRESSED) == NOTE_OFF ||
                 ((status & PRESSED) == NOTE_ON && data2 == 0))
        {
            int pressed_voices = 0;
            for (int v = 0; v < VOICES; v++)
            {
                if (synth->voices[v].pressed)
                {
                    pressed_voices++;
                }
            }
            
            for (int v = 0; v < VOICES; v++)
            {
                if (synth->voices[v].note == data1 && 
                    synth->voices[v].pressed)
                {
                    if (synth->arp && synth->voices[v].adsr->state != ENV_IDLE)
                    {
                        synth->voices[v].adsr->state = ENV_IDLE;
                    }
                    else if (!synth->arp &&
                            synth->voices[v].adsr->state != ENV_RELEASE &&
                            synth->voices[v].adsr->state != ENV_IDLE)
                    {
                        synth->voices[v].adsr->state = ENV_RELEASE;
                    }
                        
                    synth->voices[v].note = -1;
                    synth->voices[v].pressed = 0;

                    break; 
                }
            }

            if (synth->arp)
            {
                sort_synth_voices(synth);
                if (pressed_voices == 2)
                {
                    synth->active_arp_float = 1.0;
                }
            }
        }
        else if ((status & PRESSED) == KNOB_TURNED)
        {
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