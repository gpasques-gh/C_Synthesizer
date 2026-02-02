#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "defs.h"
#include "synth.h"
#include "sdl_interface.h"
#include "midi.h"
#include "kb_input.h"

void usage() {
    fprintf(stderr, "synth -kb : keyboard input, defaults to QWERTY\n");
    fprintf(stderr, "synth -kb <QWERTY/AZERTY>: keyboard input with the given keyboard layout\n");
    fprintf(stderr, "synth -midi <midi hardware id> : midi keyboard input, able to change parameters of the sounds (ADSR, cutoff, detune and oscillators waveforms)\n");
    fprintf(stderr, "use amidi -l to list your connected midi devices and find your midi device hardware id, often something like : hw:0,0,0 or hw:1,0,0\n");
    fprintf(stderr, "to see this helper again, use synth -h or synth -help\n");
}

int main(int argc, char **argv) {

    if (argc < 2) {
        usage();
        return 1;
    }

    char midi_device[256];
    snd_rawmidi_t *midi_in;

    int kb_input = 0;
    int kb_layout = QWERTY;
    int midi_input = 0;

    if (strcmp(argv[1], "-kb") == 0 && argc == 2) {
        kb_input = 1;
    } else if (strcmp(argv[1], "-kb") == 0 && argc == 3) {
        if (strcmp(argv[2], "QWERTY") == 0) {
            kb_input = 1;
            kb_layout = QWERTY;
        } else if (strcmp(argv[2], "AZERTY") == 0) {
            kb_input = 1;
            kb_layout = AZERTY;
        } else {
            usage();
            return 1;
        }
    } else if (strcmp(argv[1], "-kb") == 0 && argc > 3) {
            usage();
            return 1;
    } else if (strcmp(argv[1], "-midi") == 0 && argc >= 3) {
        midi_input = 1;
        strcpy(midi_device, argv[2]);
    } else if (strcmp(argv[1], "-midi") == 0 && argc < 3) {
        fprintf(stderr, "missing midi hardware device id.\n");
        return 1;
    } else if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
        usage();
        return 0;
    } else {
        usage();
        return 1;
    }

    int octave = DEFAULT_OCTAVE;
    note_t note = {.semitone = nC, .octave = octave, .duration = 5, .velocity = 0};

    if (kb_input) note.velocity = 127;

    lp_filter_t filter;
    lp_init(&filter, 500.0f);

    adsr_t adsr = {.att = 0.0, .dec = 0.0, .sus = 0.0, .rel = 0.0};


    osc_t voice_a_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_a_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_a_osc_c = {.phase = 0.0, .wave = 0};

    osc_t voice_b_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_b_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_b_osc_c = {.phase = 0.0, .wave = 0};

    osc_t voice_c_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_c_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_c_osc_c = {.phase = 0.0, .wave = 0};

    osc_t voice_d_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_d_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_d_osc_c = {.phase = 0.0, .wave = 0};

    osc_t voice_e_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_e_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_e_osc_c = {.phase = 0.0, .wave = 0};

    osc_t voice_f_osc_a = {.phase = 0.0, .wave = 0};
    osc_t voice_f_osc_b = {.phase = 0.0, .wave = 0};
    osc_t voice_f_osc_c = {.phase = 0.0, .wave = 0};


    synth_3osc_t voice_a =  {
        .osc_a = &voice_a_osc_a,
        .osc_b = &voice_a_osc_b,
        .osc_c = &voice_a_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    synth_3osc_t voice_b =  {
        .osc_a = &voice_b_osc_a,
        .osc_b = &voice_b_osc_b,
        .osc_c = &voice_b_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    synth_3osc_t voice_c =  {
        .osc_a = &voice_c_osc_a,
        .osc_b = &voice_c_osc_b,
        .osc_c = &voice_c_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    synth_3osc_t voice_d =  {
        .osc_a = &voice_d_osc_a,
        .osc_b = &voice_d_osc_b,
        .osc_c = &voice_d_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    synth_3osc_t voice_e =  {
        .osc_a = &voice_e_osc_a,
        .osc_b = &voice_e_osc_b,
        .osc_c = &voice_e_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    synth_3osc_t voice_f =  {
        .osc_a = &voice_f_osc_a,
        .osc_b = &voice_f_osc_b,
        .osc_c = &voice_f_osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0,
        .velocity_amplitude = 0.0,
    };

    poly_synth_t synth = {
        .voice_a = &voice_a,
        .voice_b = &voice_b,
        .voice_c = &voice_c,
        .voice_d = &voice_d,
        .voice_e = &voice_e,
        .voice_f = &voice_f,
        .amplitude = DEFAULT_AMPLITUDE,
        .lp_filter = &filter,
        .detune = 0.0
    };

    snd_pcm_t *handle;

    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        perror("snd_pcm_open");
        return 1;
    }

    int params_err = snd_pcm_set_params(handle,
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        1, RATE, 1, LATENCY);

    if (params_err < 0) {
        fprintf(stderr, "snd_pcm_set_params error: %s\n", snd_strerror(params_err));
        return 1;
    }

    snd_pcm_prepare(handle);

    short buffer[FRAMES];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < FRAMES; j++) {
            buffer[j] = 0;
        }
        snd_pcm_writei(handle, buffer, FRAMES);
    }

    SDL_Window *window = SDL_CreateWindow("Awesome Synth!", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        fprintf(stderr, "SDL window creation error.\n");
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "SDL renderer creation error\n");
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Init();
    TTF_Font* sans = TTF_OpenFont("Regular.ttf", 24);

    if (midi_input) {
        snd_rawmidi_open(&midi_in, NULL, midi_device, SND_RAWMIDI_NONBLOCK);
        if (!midi_in) {
            fprintf(stderr, "error while opening midi device %s, check your midi devices using amidi -l\n", midi_device);
            return 1;
        }
    }

    SDL_Event event;

    int running = 1;

    int n_voices = 0;

    while (running) {

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                running = 0;
            }
            /*if (kb_input && event.type == SDL_KEYDOWN) {
                handle_input(&event, &note, &synth_3osc, kb_layout);
            }*/
        }

        if (midi_input)
            get_midi(midi_in, &note, &synth, &n_voices);

        render_poly_synth(&synth, buffer, n_voices);
        //fprintf(stderr, "Voices : %d", n_voices);
        int err = snd_pcm_writei(handle, buffer, FRAMES);
        if (err == -EPIPE) {
            snd_pcm_prepare(handle);
        } else if (err < 0) {
            snd_pcm_prepare(handle);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        render_interface(note, voice_a, sans, renderer);
        render_waveform(renderer, buffer);
        SDL_RenderPresent(renderer);
    }

    if (midi_input) snd_rawmidi_close(midi_in);
    TTF_CloseFont(sans);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = NULL;
    window = NULL;
    SDL_Quit();

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    return 0;
}
