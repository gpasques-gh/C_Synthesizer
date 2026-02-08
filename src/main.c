#include <math.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define RAYGUI_IMPLEMENTATION

#include "interface.h"
#include "synth.h"
#include "midi.h"
#include "keyboard.h"

/* Prints the usage of the CLI arguments into the error output */
void usage()
{
    fprintf(stderr, "synth -kb : keyboard input, defaults to QWERTY\n");
    fprintf(stderr, "synth -kb <QWERTY/AZERTY>: keyboard input with the given keyboard layout\n");
    fprintf(stderr, "synth -midi <midi hardware id> : midi keyboard input, able to change parameters of the sounds (ADSR, cutoff, detune and oscillators waveforms)\n");
    fprintf(stderr, "use amidi -l to list your connected midi devices and find your midi device hardware id, often something like : hw:0,0,0 or hw:1,0,0\n");
    fprintf(stderr, "to see this helper again, use synth -h or synth -help\n");
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 1;
    }

    snd_pcm_t *handle = NULL;
    snd_rawmidi_t *midi_in = NULL;

    char midi_device[256];
    int midi_input = 0;
    int keyboard_input = 0;
    int keyboard_layout = QWERTY;

    if (strcmp(argv[1], "-kb") == 0 && argc == 2)
        keyboard_input = 1;
    else if (strcmp(argv[1], "-kb") == 0 && argc == 3)
    {
        if (strcmp(argv[2], "QWERTY") == 0)
            keyboard_input = 1;
        else if (strcmp(argv[2], "AZERTY") == 0)
        {
            keyboard_input = 1;
            keyboard_layout = AZERTY;
        }
        else
        {
            usage();
            return 1;
        }
    }
    else if (strcmp(argv[1], "-kb") == 0 && argc > 3)
    {
        usage();
        return 1;
    }
    else if (strcmp(argv[1], "-midi") == 0 && argc >= 3)
    {
        midi_input = 1;
        strncpy(midi_device, argv[2], sizeof(midi_device) - 1);
        midi_device[sizeof(midi_device) - 1] = '\0';
    }
    else if (strcmp(argv[1], "-midi") == 0 && argc < 3)
    {
        fprintf(stderr, "missing midi hardware device id. \n");
        return 1;
    }
    else
    {
        usage();
        return 1;
    }

    int octave = DEFAULT_OCTAVE;

    int osc_a = SINE_WAVE, osc_b = SINE_WAVE, osc_c = SINE_WAVE;

    float attack = 0.2;
    float decay = 0.3;
    float sustain = 0.7;
    float release = 0.2;

    float filter_attack = 0.0;
    float filter_decay = 0.3;
    float filter_sustain = 0.0;
    float filter_release = 0.2;

    adsr_t filter_adsr =
    {
        .attack = &filter_attack,
        .decay = &filter_decay,
        .sustain = &filter_sustain,
        .release = &filter_release
    };

    lp_filter_t filter =
    {
        .cutoff = 0.5,
        .prev_input = 0.0,
        .prev_output = 0.0,
        .adsr = &filter_adsr,
        .env = false
    };

    synth_t synth =
        {
            .voices = malloc(sizeof(voice_t) * VOICES),
            .amp = DEFAULT_AMPLITUDE,
            .detune = 0.0,
            .filter = &filter};

    if (synth.voices == NULL)
    {
        fprintf(stderr, "memory allocation failed.\n");
        return 1;
    }

    for (int i = 0; i < VOICES; i++)
    {
        synth.voices[i].adsr = malloc(sizeof(adsr_t));
        if (synth.voices[i].adsr == NULL)
        {
            fprintf(stderr, "memory allocation failed.\n");
            for (int j = 0; j < i; j++)
            {
                free(synth.voices[j].adsr);
                free(synth.voices[j].oscillators);
            }
            free(synth.voices);
            return 1;
        }

        synth.voices[i].adsr->attack = &attack;
        synth.voices[i].adsr->decay = &decay;
        synth.voices[i].adsr->sustain = &sustain;
        synth.voices[i].adsr->release = &release;
        synth.voices[i].adsr->state = ENV_IDLE;
        synth.voices[i].adsr->output = 0.0;
        synth.voices[i].note = 0;
        synth.voices[i].velocity_amp = 0.0;
        synth.voices[i].active = 0;

        synth.voices[i].oscillators = malloc(sizeof(osc_t) * 3);
        if (synth.voices[i].oscillators == NULL)
        {
            fprintf(stderr, "memory allocation failed.\n");
            goto cleanup_synth;
        }
        for (int j = 0; j < 3; j++)
        {
            synth.voices[i].oscillators[j].freq = 0.0;
            synth.voices[i].oscillators[j].phase = 0.0;
        }
        synth.voices[i].oscillators[0].wave = &osc_a;
        synth.voices[i].oscillators[1].wave = &osc_b;
        synth.voices[i].oscillators[2].wave = &osc_c;
    }

    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        fprintf(stderr, "error while opening sound card.\n");
        goto cleanup_synth;
    }

    int params_err = snd_pcm_set_params(
        handle,
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        1, RATE, 1, LATENCY);

    if (params_err < 0)
    {
        fprintf(stderr, "error while setting sound card parameters: %s\n", snd_strerror(params_err));
        goto cleanup_alsa;
    }

    if (midi_input)
        if (snd_rawmidi_open(&midi_in, NULL, midi_device, SND_RAWMIDI_NONBLOCK) < 0)
        {
            fprintf(stderr, "error while opening midi device %s\n", midi_device);
            goto cleanup_alsa;
        }

    snd_pcm_prepare(handle);

    short buffer[FRAMES];
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < FRAMES; j++)
            buffer[j] = 0;
        snd_pcm_writei(handle, buffer, FRAMES);
    }

    bool ddm_a = false, ddm_b = false, ddm_c = false, saving_preset = false;
    char filename[1024] = "\0";
    
    InitWindow(WIDTH, HEIGHT, "ALSA & raygui synthesizer");
    Font annotation = LoadFont("Regular.ttf");
    GuiSetFont(annotation);
    GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize * 0.5); 

    while (!WindowShouldClose())
    {
        if (keyboard_input && !saving_preset)
        {
            handle_input(&synth, keyboard_layout, &octave);
            handle_release(&synth, keyboard_input, octave);
        }

        if (midi_input)
            get_midi(midi_in, &synth, &attack, &decay, &sustain, &release);

        render_synth(&synth, buffer);

        int err = snd_pcm_writei(handle, buffer, FRAMES);
        if (err == -EPIPE)
        {
            fprintf(stderr, "ALSA underrun!\n");
            snd_pcm_prepare(handle);
        }
        else if (err < 0)
        {
            fprintf(stderr, "ALSA write error: %s\n", snd_strerror(err));
            snd_pcm_prepare(handle);
        }

        BeginDrawing();
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            render_waveform(buffer);
            render_informations(
                &synth,
                &attack, &decay, &sustain, &release,
                &osc_a, &osc_b, &osc_c,
                &ddm_a, &ddm_b, &ddm_c,
                filename, &saving_preset);
            render_white_keys();
            for (int v = 0; v < VOICES; v++)
                if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && !is_black_key(synth.voices[v].note))
                    render_key(synth.voices[v].note);

            render_black_keys();
            for (int v = 0; v < VOICES; v++)
                if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && is_black_key(synth.voices[v].note))
                    render_key(synth.voices[v].note);
        EndDrawing();
    }

    CloseWindow();

    if (midi_in)
        snd_rawmidi_close(midi_in);

cleanup_alsa:
    if (handle)
    {
        snd_pcm_drain(handle);
        snd_pcm_close(handle);
    }
cleanup_synth:
    for (int i = 0; i < VOICES; i++)
    {
        free(synth.voices[i].adsr);
        free(synth.voices[i].oscillators);
    }
    free(synth.voices);

    return 0;
}