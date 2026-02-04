#define _GNU_SOURCE
#include <math.h>
#include "defs.h"
#include "synth.h"

double adsr_process(adsr_t *adsr)
{
    switch (adsr->state)
    {
    case ENV_IDLE:
        return 0.0;
        break;
    case ENV_ATTACK:
        if (*adsr->attack > 0.0)
        {
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
            double decrement = (1.0 - *adsr->sustain) / (*adsr->decay * RATE);
            adsr->output -= decrement;
            if (adsr->output <= *adsr->sustain)
            {
                adsr->output = *adsr->sustain;
                adsr->state = ENV_SUSTAIN;
            }
        }
        else
        {
            adsr->output = *adsr->sustain;
            adsr->state = ENV_SUSTAIN;
        }
        break;
    case ENV_SUSTAIN:
        adsr->output = *adsr->sustain;
        break;
    case ENV_RELEASE:
        if (*adsr->release > 0.0)
        {
            double decrement = adsr->output / (*adsr->release * RATE);
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

void render_synth(synth_t *synth, short *buffer)
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
        {
            double envelope = adsr_process(voice->adsr);
            double mixed = 0.0;

            for (int o = 0; o < 3; o++)
            {
                osc_t *osc = &voice->oscillators[o];
                double phase_inc = osc->freq / RATE;
                double sample;

                switch (osc->wave)
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

    double gain = (active_voices > 0)
                      ? 1.0 / sqrt((double)active_voices)
                      : 0.0;

    for (int i = 0; i < FRAMES; i++)
    {
        double sample = temp_buffer[i] * gain;
        if (sample > 1.0)
            sample = 1.0;
        if (sample < -1.0)
            sample = -1.0;

        short sample_short = (short)(sample * 32767.0);
        sample_short = lp_process(synth->filter, sample_short);
        buffer[i] = sample_short;
    }
}

void change_freq(voice_t *voice, int note, int velocity, double detune)
{
    int a4_diff = note - A4_POSITION;

    voice->note = note;
    voice->active = 1;
    voice->adsr->output = 0.001;
    voice->adsr->state = ENV_ATTACK;
    voice->velocity_amp = velocity / MIDI_MAX_VALUE;

    voice->oscillators[0].freq = A_4 * pow(2, a4_diff / 12.0);
    voice->oscillators[0].phase = 0.0;
    voice->oscillators[1].freq = A_4 * pow(2, a4_diff / 12.0) + (5 * detune);
    voice->oscillators[1].phase = 0.0;
    voice->oscillators[2].freq = A_4 * pow(2, a4_diff / 12.0) - (5 * detune);
    voice->oscillators[2].phase = 0.0;
}

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

void lp_init(lp_filter_t *filter, float cutoff)
{
    float rc = 1.0f / (2.0f * M_PI * cutoff);
    float dt = 1.0f / RATE;
    filter->alpha = dt / (rc + dt);
    filter->prev = 0.0f;
    filter->cutoff = cutoff;
}

short lp_process(lp_filter_t *filter, short input)
{
    float x = (float)input;
    filter->prev = filter->prev + filter->alpha * (x - filter->prev);
    return (short)filter->prev;
}

voice_t *get_free_voice(synth_t *synth)
{
    for (int i = 0; i < VOICES; i++)
        if (!synth->voices[i].active)
            return &synth->voices[i];
    return NULL;
}