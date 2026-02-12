#define _GNU_SOURCE
#include <math.h>

#include "defs.h"
#include "synth.h"

/*
 * Process a sample from the ADSR envelope
 * Returns the envelope amplification coeficient
 */
float adsr_process(adsr_t *adsr)
{
    switch (adsr->state)
    {
    case ENV_IDLE:
        return 0.0;
        break;
    case ENV_ATTACK:
        if (*adsr->attack > 0.0)
        {   /* Increment the amplification by the attack amount */
            double increment = 1.0 / (*adsr->attack * RATE);
            adsr->output += increment;
            if (adsr->output >= 1.0)
            {
                adsr->output = 1.0;
                adsr->state = ENV_DECAY;
            }
        }
        else
        {   /* If no attack, go in decay */
            adsr->output = 1.0;
            adsr->state = ENV_DECAY;
        }
        break;
    case ENV_DECAY:
        if (*adsr->decay > 0.0)
        {
            if (*adsr->sustain > 0.0)
            {   /* Decrement the amplification by the decay amount relatively to the sustain amount */
                float decrement = (1.0 - *adsr->sustain) / (*adsr->decay * RATE);
                adsr->output -= decrement;

                if (adsr->output <= *adsr->sustain)
                {
                    adsr->output = *adsr->sustain;
                    adsr->state = ENV_SUSTAIN;
                }
            }
            else
            {   /* Decrement the amplification by the decay amount relatively to the release amount */
                float decrement = (1.0 - *adsr->release) / (*adsr->decay * RATE);
                adsr->output -= decrement;

                if (adsr->output <= *adsr->release && adsr->release)
                {
                    adsr->output = *adsr->release;
                    adsr->state = ENV_RELEASE;
                }
            }
        }
        else
        {   /* If there is sustain, go in sustain */
            if (*adsr->sustain > 0.0)
            {
                adsr->output = *adsr->sustain;
                adsr->state = ENV_SUSTAIN;
            }
            /* Else go in release */
            else
            {
                adsr->output = *adsr->release;
                adsr->state = ENV_RELEASE;
            }
        }
        break;
    case ENV_SUSTAIN:
        if (*adsr->sustain == 0.0)
        {   /* Increment the amplification by the attack amount */
            float decrement = adsr->output / (*adsr->release * RATE);
            adsr->output -= decrement;
            adsr->state = ENV_RELEASE;
        }
        else
        {
            /* We put the amplification at the sustain level */
            adsr->output = *adsr->sustain;
        }
        break;
    case ENV_RELEASE:
        if (*adsr->release > 0.0)
        {   /* Decrement the amplification by the release amount */
            float decrement = adsr->output / (*adsr->release * RATE);
            adsr->output -= decrement;
            if (adsr->output <= 0.001)
            {
                adsr->output = 0.0;
                adsr->state = ENV_IDLE;
            }
        }
        else
        {   /* If no release, go in idle state */
            adsr->output = 0.0;
            adsr->state = ENV_IDLE;
        }
        break;
    }
    /* Return the amplification of the ADSR envelope */
    return adsr->output;
}


/* Process the synth voices into the sound buffer */
void process_voices(synth_t *synth, double *tmp_buffer, int *active_voices)
{
    *active_voices = 0;

    for (int v = 0; v < VOICES; v++)
    {
        voice_t *voice = &synth->voices[v];
        if (voice->adsr->state == ENV_IDLE)
        {
            continue;
        }
        (*active_voices)++;

        if ((synth->arp && v == synth->active_arp) || !synth->arp)
        {
            for (int i = 0; i < FRAMES; i++)
            {
                float envelope = adsr_process(voice->adsr);
                double mixed = 0.0;

                for (int o = 0; o < 3; o++)
                {
                    osc_t *osc = &voice->oscillators[o];
                    double phase_inc = osc->freq / RATE;
                    double sample;

                    switch (*osc->wave)
                    {
                    case SINE_WAVE:
                        sample = sin(2.0 * M_PI * osc->phase);
                        break;
                    case SQUARE_WAVE:
                        sample = (osc->phase < 0.5) ? 1.0 : -1.0;
                        break;
                    case TRIANGLE_WAVE:
                        sample = 1.0 - 4.0 * fabs(osc->phase - 0.5);
                        break;
                    case SAWTOOTH_WAVE:
                        sample = 2.0 * osc->phase - 1.0;
                        break;
                    default:
                        sample = 0.0;
                        break;
                    }

                    mixed += sample;

                    osc->phase += phase_inc;
                    if (osc->phase >= 1.0)
                    {
                        osc->phase -= 1.0;
                    }
                }

                /* Oscillator sound mix */
                mixed /= 3.0;
                mixed *= envelope;
                mixed *= voice->velocity_amp;

                if (synth->lfo->mod_param == LFO_AMP)
                {
                    mixed *= synth->lfo_amp;
                }
                else
                {
                    mixed *= synth->amp;
                }

                tmp_buffer[i] += mixed;
            }
        }
    }
}

/* Process the LFO modulation */
void process_lfo(synth_t *synth)
{
    if (synth->lfo->mod_param != LFO_OFF)
    {
        /* Processing the LFO */
        double phase_inc = synth->lfo->osc->freq / RATE;
        double automation;
        
        /* Calculating the wave from the LFO */
        switch (*synth->lfo->osc->wave)
        {
        case SINE_WAVE:
            automation = fabs(sin(2.0 * M_PI * synth->lfo->osc->phase));
            break;
        case SQUARE_WAVE:
            automation = (synth->lfo->osc->phase < 0.5) ? 1.0 : 0.0;
            break;
        case TRIANGLE_WAVE:
            automation = fabs(1.0 - 4.0 * fabs(synth->lfo->osc->phase - 0.5));
            break;
        case SAWTOOTH_WAVE:
            automation = synth->lfo->osc->phase;
            break;
        default:
            automation = 0.0;
            break;
        }

        /* Applyging the LFO to the assigned parameter */
        switch (synth->lfo->mod_param)
        {
        case LFO_CUTOFF:
            synth->filter->lfo_cutoff = synth->filter->cutoff * automation;
            break;
        case LFO_DETUNE:
            synth->lfo_detune = synth->detune * automation;
            apply_detune_change(synth);
            break;
        case LFO_AMP:
            synth->lfo_amp = synth->amp * automation;
            break;
        default:
            break;
        }

        synth->lfo->osc->phase += phase_inc;
        if (synth->lfo->osc->phase >= 1.0)
        {
            synth->lfo->osc->phase -= 1.0;
        }
    }
}

/* Process the gain and low-pass filter onto the sound buffer */
double process_gain(synth_t synth, double sample, int active_voices)
{
    /* No gain if arpeggio*/
    if (synth.arp)
    {
        return sample;
    }
    
     /* Gain to stay at the same level despite the number of active voices */
    double gain = (active_voices > 0)
                      ? 1.0 / sqrt((double)active_voices)
                      : 0.0;

    /* Gain processing */
    double processed_sample = sample * gain;
    if (processed_sample > 1.0)
    {
        processed_sample = 1.0;
    }
    if (processed_sample < -1.0)
    {
        processed_sample = -1.0;
    }

    return processed_sample;
}

double process_filter(synth_t *synth, double sample)
{
    double cutoff = synth->filter->cutoff;
 
    if (synth->filter->env)
    {
        cutoff = synth->filter->cutoff +
                        adsr_process(synth->filter->adsr) / 2;
        if (cutoff > 1.0)
        {
            cutoff = 1.0;
        }
        synth->filter->env_cutoff = cutoff;
    }

    /* If the LFO is on the filter, override the filter envelope */
    if (synth->lfo->mod_param == LFO_CUTOFF)
    {
        cutoff = synth->filter->lfo_cutoff;
    }
    return lp_process(synth->filter, sample, cutoff);
}

/* Process the arpeggiator */
void process_arpeggiator(synth_t *synth, int active_voices)
{
    if (synth->arp)
    {
        float bpm_increment = 1.0 / (60.0 / (float)synth->bpm * RATE);
        synth->active_arp_float += bpm_increment;

        if (synth->active_arp_float >= 1.0)
        {
            synth->active_arp++;
            if (synth->active_arp >= active_voices)
            {
                synth->active_arp = 0;
            }
                

            synth->active_arp_float = 0.0;
            
            /* Reseting ADSR envelope */
            if (synth->voices[synth->active_arp].pressed)
            {
                synth->voices[synth->active_arp].adsr->state = ENV_ATTACK;
                if (synth->filter->env)
                {
                    synth->filter->adsr->state = ENV_ATTACK;
                }
            }

            /* debug logs */
            //fprintf(stderr, "\nfirst note %d\n", synth->voices[0].note);
            //fprintf(stderr, "active voices %d\n", active_voices);
            //fprintf(stderr, "active arp %d\n", synth->active_arp);
        }
    }
}

/*
 * Change the frequency of a voice_t oscillators with the given MIDI note and velocity
 * Multiplied by the synth_t detune coefficient
 */
void change_freq(voice_t *voice, int note,
                 int velocity, double detune)
{
    int a4_diff = note - A4_POSITION;

    /* Activating the voice */
    voice->note = note;
    voice->adsr->output = 0.001;
    voice->adsr->state = ENV_ATTACK;
    voice->velocity_amp = velocity / MIDI_MAX_VALUE;

    /* Applying the frequency and detune effect to the oscillators */
    voice->oscillators[0].freq = A_4 * pow(2, a4_diff / 12.0);
    voice->oscillators[0].phase = 0.0;
    voice->oscillators[1].freq = A_4 * pow(2, a4_diff / 12.0) + (5 * detune);
    voice->oscillators[1].phase = 0.0;
    voice->oscillators[2].freq = A_4 * pow(2, a4_diff / 12.0) - (5 * detune);
    voice->oscillators[2].phase = 0.0;
}

/* Apply the detune change to the voices oscillators */
void apply_detune_change(synth_t *synth)
{
    float detune;
    if (synth->lfo->mod_param == LFO_DETUNE)
    {
        detune = synth->lfo_detune;
    }
    else 
    {
        detune = synth->detune;
    }
    
    for (int v = 0; v < VOICES; v++)
    {
        int a4_diff = synth->voices[v].note - A4_POSITION;
        synth->voices[v].oscillators[1].freq = A_4 * pow(2, a4_diff / 12.0) + (5 * detune);
        synth->voices[v].oscillators[2].freq = A_4 * pow(2, a4_diff / 12.0) - (5 * detune);
    }
}

/* Get the literal name of a given waveform */
const char *get_wave_name(int wave)
{
    switch (wave)
    {
    case SINE_WAVE:
        return "Sine wave";
    case SQUARE_WAVE:
        return "Square wave";
    case TRIANGLE_WAVE:
        return "Triangle wave";
    case SAWTOOTH_WAVE:
        return "Sawtooth wave";
    default:
        return "Unknown wave";
    }
}

/*
 * Process a sample with the low-pass filter and the given cutoff
 * Returns the processed sample
 */
double lp_process(lp_filter_t *filter, double input,
                float cutoff)
{
    /* Clipping */
    if (cutoff > 1.0f)
    {
        cutoff = 1.0f;
    }
    if (cutoff < 0.0f)
    {
        cutoff = 0.0f;
    }

    /* Calculating the filter amplification */
    float frequency = cutoff * (RATE / 8.0f);
    float omega = 2.0f * M_PI * frequency / RATE;
    float alpha = omega / (omega + 1.0f);
    float input_f = (float)input;
    float output = alpha * input_f + (1.0f - alpha) * filter->prev_output;

    /* Setting the previous output and input of the filter */
    filter->prev_output = output;
    filter->prev_input = input_f;

    return (double)output;
}

/*
 * Returns the first free voice from the synth_t
 * Used to assign a note send by MIDI or keyboard to the first free voice
 */
voice_t *get_free_voice(synth_t *synth)
{
    for (int i = 0; i < VOICES; i++)
    {
        if (!synth->arp && synth->voices[i].adsr->state == ENV_IDLE)
        {
            return &synth->voices[i];
        }
        else if (synth->arp && !synth->voices[i].pressed)
        {
            return &synth->voices[i];
        }
    }
    return NULL;
}

/* Insertion sort algorithm for the voices of a synth_t, used for arpeggiator */
void sort_synth_voices(synth_t *synth)
{
    /* First sort the voices */
    for (int v = 1; v < VOICES; v++)
    {
        voice_t current = synth->voices[v];
        int i = v - 1;
        
        while (i >= 0 && synth->voices[i].note > current.note)
        {
            synth->voices[i + 1] = synth->voices[i];
            i--;
        }

        synth->voices[i + 1] = current;
    }

    /* Then move the voices with empty notes to the end */
    for (int v = 1; v < VOICES; v++)
    {
        voice_t current = synth->voices[v];
        int i = v - 1;
        
        while (i >= 0 && synth->voices[i].note == -1)
        {
            synth->voices[i + 1] = synth->voices[i];
            i--;
        }

        synth->voices[i + 1] = current;
    }
}
