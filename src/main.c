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

int main(int argc, char **argv) {

    int octave = DEFAULT_OCTAVE;
    note_t note = {.semitone = nC, .octave = octave, .duration = 5};
    
    int osc_a_wave = 0;
    int osc_b_wave = 0;
    int osc_c_wave = 0;

    double attack = 0.05;
    double decay = 0.2;
    double sustain = 0.7;
    double release = 0.2;

    char midi_device[256];

    if (argc >= 2) {
        strcpy(midi_device, argv[1]);
    }

    adsr_t adsr = {
        .att = attack,
        .dec = decay,
        .sus = sustain,
        .rel = release
    };

    osc_t osc_a = {
        .active = 0,
        .phase = 0.0,
        .frames_left = 0,
        .frames_total = 0,
        .wave = osc_a_wave
    };

    osc_t osc_b = {
        .active = 0,
        .phase = 0.0,
        .frames_left = 0,
        .frames_total = 0,
        .wave = osc_b_wave
    };

    osc_t osc_c = {
        .active = 0,
        .phase = 0.0,
        .frames_left = 0,
        .frames_total = 0,
        .wave = osc_c_wave
    };

    synth_3osc_t synth_3osc =  {
        .osc_a = &osc_a,
        .osc_b = &osc_b,
        .osc_c = &osc_c,
        .adsr = &adsr,
        .active = 0,
        .frames_left = 0.0,
        .frames_total = 0.0
    };

    snd_pcm_t *handle;
        
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        perror("snd_pcm_open");
        return 1;
    }

    int params_err = snd_pcm_set_params(handle, 
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        1, RATE, 1, 50000);

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
        perror("SDL window creation error.");
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        perror("SDL renderer creation error.");
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    TTF_Init();
    TTF_Font* sans = TTF_OpenFont("FreeSans.ttf", 24);

    snd_rawmidi_t *midi_in;
    snd_rawmidi_open(&midi_in, NULL, midi_device, SND_RAWMIDI_NONBLOCK);

    if (!midi_in) {
        printf("Erreur midi");
        return 1;
    }

    SDL_Event event;

    int running = 1;

    while (running) {

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                running = 0;
            } 
        }

        int midi_err = get_midi(midi_in, &note, &synth_3osc);
    
        render_synth3osc(&synth_3osc, buffer);
        int err = snd_pcm_writei(handle, buffer, FRAMES);
        if (err == -EPIPE) {
            snd_pcm_prepare(handle);
        } else if (err < 0) {
            snd_pcm_prepare(handle);
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        render_interface(note, synth_3osc, sans, renderer);
        SDL_RenderPresent(renderer);
        
    }

    snd_rawmidi_close(midi_in);
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
