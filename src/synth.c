#define _GNU_SOURCE
#include <math.h>
#include "defs.h"
#include "synth.h"

/*
 * Process a sample from the ADSR envelope
 * Returns the envelope amplification coeficient
 */
float 
adsr_process(adsr_t *adsr)
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
        {
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
        {
            if (*adsr->sustain > 0.0)
            {
                adsr->output = *adsr->sustain;
                adsr->state = ENV_SUSTAIN;
            }
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
        else adsr->output = *adsr->sustain;
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
        {
            adsr->output = 0.0;
            adsr->state = ENV_IDLE;
        }
        break;
    }
    return adsr->output;
}

/* Renders the synth_t voices into the temporary sound buffer */
void 
render_synth(synth_t *synth, short *buffer)
{
    double temp_buffer[FRAMES];
    memset(temp_buffer, 0, FRAMES * sizeof(double));

    int active_voices = 0;

    for (int v = 0; v < VOICES; v++)
    {
        voice_t *voice = &synth->voices[v];
        if (!voice->active)
            continue;
        active_voices++;

        for (int i = 0; i < FRAMES; i++)
        { /* Oscillators processing for each voice*/
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
                }

                mixed += sample;

                osc->phase += phase_inc;
                if (osc->phase >= 1.0)
                    osc->phase -= 1.0;
            }

            /* Oscillator sound mix */
            mixed /= 3.0;
            mixed *= envelope;
            mixed *= voice->velocity_amp;
            mixed *= synth->amp;

            temp_buffer[i] += mixed;
        }

        if (voice->adsr->state == ENV_IDLE)
        {
            voice->active = 0;
            if (active_voices > 0)
                active_voices--;
        }
    }

    /* Gain to stay at the same level despite the number of active voices */
    double gain = (active_voices > 0)
                      ? 1.0 / sqrt((double)active_voices)
                      : 0.0;

    for (int i = 0; i < FRAMES; i++)
    { /* Low-pass filter and gain processing */
        
        double env_cutoff = synth->filter->cutoff;
        if (synth->filter->env)
        {
            env_cutoff = synth->filter->cutoff + 
            adsr_process(synth->filter->adsr) / 2;
            if (env_cutoff > 1.0) env_cutoff = 1.0;
        }
        
        double sample = temp_buffer[i] * gain;
        if (sample > 1.0)
            sample = 1.0;
        if (sample < -1.0)
            sample = -1.0;

        sample = lp_process(synth->filter, sample, env_cutoff);
        synth->filter->env_cutoff = env_cutoff;
        buffer[i] = (short)(sample * 32767.0);;
    }
}

/*
 * Change the frequency of a voice_t oscillators with the given MIDI note and velocity
 * Multiplied by the synth_t detune coefficient
 */
void 
change_freq(voice_t *voice, int note, 
            int velocity, double detune)
{
    int a4_diff = note - A4_POSITION;

    /* Activating the voice */
    voice->note = note;
    voice->active = 1;
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
void 
apply_detune_change(synth_t *synth) 
{
    for (int v = 0; v < VOICES; v++)
    { /* Applying detune changes to each voice of the synthesizer */
        int a4_diff = synth->voices[v].note - A4_POSITION;
        synth->voices[v].oscillators[1].freq = A_4 * pow(2, a4_diff / 12.0) + (5 * synth->detune);
        synth->voices[v].oscillators[2].freq = A_4 * pow(2, a4_diff / 12.0) - (5 * synth->detune);
    }
}

/* Get the literal name of a given waveform */
const char 
*get_wave_name(int wave)
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
 * Process a sample with the low-pass filter
 * Returns the processed sample
 */
double 
lp_process(lp_filter_t *filter, double input, 
            float cutoff)
{
    if (cutoff > 1.0f) cutoff = 1.0f;
    if (cutoff < 0.0f) cutoff = 0.0f;

    float frequency = cutoff * (RATE / 8.0f);
    float omega = 2.0f * M_PI * frequency / RATE;
    float alpha = omega / (omega + 1.0f);
    
    float input_f = (float)input;
    float output = alpha * input_f + (1.0f - alpha) * filter->prev_output;
    
    filter->prev_output = output;
    filter->prev_input = input_f;
    
    return (double)output;
}

/*
 * Returns the first free voice from the synth_t
 * Used to assign a note send by MIDI or keyboard to the first free voice
 */
voice_t 
*get_free_voice(synth_t *synth)
{
    for (int i = 0; i < VOICES; i++)
        if (!synth->voices[i].active)
            return &synth->voices[i];
    return NULL;
}
