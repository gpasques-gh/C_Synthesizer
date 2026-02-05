#include <math.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "defs.h"
#include "synth.h"
#include "midi.h"
#include "keyboard.h"
#include "interface.h"

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
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
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

    double attack = 0.2;
    double decay = 0.3;
    double sustain = 0.7;
    double release = 0.2;

    double filter_attack = 0.0;
    double filter_decay = 0.3;
    double filter_sustain = 0.0;
    double filter_release = 0.2;

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
        .adsr = &filter_adsr
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
            synth.voices[i].oscillators[j].wave = SINE_WAVE;
        }
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

    snd_pcm_prepare(handle);

    short buffer[FRAMES];
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < FRAMES; j++)
            buffer[j] = 0;
        snd_pcm_writei(handle, buffer, FRAMES);
    }

    window = SDL_CreateWindow(
        "ALSA Synth", 0, 0,
        WIDTH, HEIGHT,
        SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        fprintf(stderr, "error while creating SDL window.\n");
        goto cleanup_alsa;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        fprintf(stderr, "error while creating SDL renderer.\n");
        goto cleanup_window;
    }

    SDL_Texture *white_keys_texture = SDL_CreateTexture(renderer,
                                                        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                        WIDTH, WHITE_KEYS_HEIGHT);

    if (white_keys_texture == NULL)
    {
        fprintf(stderr, "error while creating keyboard texture: %s\n", SDL_GetError());
        goto cleanup_renderer;
    }

    SDL_SetTextureBlendMode(white_keys_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, white_keys_texture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    render_white_keys(renderer);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_Texture *black_keys_texture = SDL_CreateTexture(renderer,
                                                        SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                                        WIDTH, WHITE_KEYS_HEIGHT);

    if (black_keys_texture == NULL)
    {
        fprintf(stderr, "error while creating keyboard texture: %s\n", SDL_GetError());
        goto cleanup_white_keys_texture;
    }

    SDL_SetTextureBlendMode(black_keys_texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, black_keys_texture);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    render_black_keys(renderer);
    SDL_SetRenderTarget(renderer, NULL);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Event event;

    TTF_Init();
    font = TTF_OpenFont("Regular.ttf", 24);
    if (font == NULL)
    {
        fprintf(stderr, "error while loading font: %s\n", TTF_GetError());
        goto cleanup_black_keys_texture;
    }

    if (midi_input)
        if (snd_rawmidi_open(&midi_in, NULL, midi_device, SND_RAWMIDI_NONBLOCK) < 0)
        {
            fprintf(stderr, "error while opening midi device %s\n", midi_device);
            goto cleanup_font;
        }

    int running = 1;
    while (running)
    {
        while (SDL_PollEvent(&event) != 0)
        {
            if (event.type == SDL_QUIT)
                running = 0;
            else if (keyboard_input &&
                     event.type == SDL_KEYDOWN &&
                     event.key.repeat == 0)
                handle_input(event.key.keysym.sym, &synth, keyboard_layout,
                             &octave, &attack, &decay, &sustain, &release);
            else if (keyboard_input && event.type == SDL_KEYUP)
                handle_release(event.key.keysym.sym, &synth, keyboard_layout, octave);
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

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect outside_key = {.w = WIDTH, .h = HEIGHT - WHITE_KEYS_HEIGHT, .x = 0, .y = 0};
        SDL_RenderFillRect(renderer, &outside_key);

        render_infos(synth, font, renderer, attack, decay, sustain, release);
        render_waveform(renderer, buffer);

        SDL_Rect keyboard_dest = {0, HEIGHT - WHITE_KEYS_HEIGHT, WIDTH, WHITE_KEYS_HEIGHT};
        SDL_RenderCopy(renderer, white_keys_texture, NULL, &keyboard_dest);

        for (int v = 0; v < VOICES; v++)
            if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && !is_black_key(synth.voices[v].note))
                render_key(renderer, synth.voices[v].note);

        SDL_RenderCopy(renderer, black_keys_texture, NULL, &keyboard_dest);

        for (int v = 0; v < VOICES; v++)
            if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && is_black_key(synth.voices[v].note))
                render_key(renderer, synth.voices[v].note);

        SDL_RenderPresent(renderer);
    }

    if (midi_in)
        snd_rawmidi_close(midi_in);

    cleanup_text_cache();

cleanup_font:
    if (font)
        TTF_CloseFont(font);
    TTF_Quit();
cleanup_black_keys_texture:
    if (black_keys_texture)
        SDL_DestroyTexture(black_keys_texture);
cleanup_white_keys_texture:
    if (white_keys_texture)
        SDL_DestroyTexture(white_keys_texture);
cleanup_renderer:
    if (renderer)
        SDL_DestroyRenderer(renderer);
cleanup_window:
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
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