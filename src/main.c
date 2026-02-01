#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "defs.h"
#include "synth.h"
#include "input.h"
#include "sdl_interface.h"
#include "midi.h"

int main(int argc, char **argv) {

    
    

    int octave = DEFAULT_OCTAVE;
    note_t note = {.n_semitone = nC, .n_octave = octave, .n_duration = 5};
    
    int osc_a_wave = 0;
    int osc_b_wave = 0;
    int osc_c_wave = 0;

    double attack = 0.05;
    double decay = 0.2;
    double sustain = 0.7;
    double release = 0.2;

    if (argc >= 8) {
        attack = atof(argv[1]);
        decay = atof(argv[2]);
        sustain = atof(argv[3]);
        release = atof(argv[4]);
        osc_a_wave = atol(argv[5]);
        osc_b_wave = atol(argv[6]);
        osc_c_wave = atol(argv[7]);
    }

    adsr_t adsr = {
        .att = attack,
        .dec = decay,
        .sus = sustain,
        .rel = release
    };

    sound_t sound = {
        .s_adsr = &adsr,
        .s_active = 0,
        .s_phase = 0.0,
        .s_frames_left = 0,
        .s_frames_total = 0,
        .s_wave = osc_a_wave
    };

    sound_t sound_b = {
        .s_adsr = &adsr,
        .s_active = 0,
        .s_phase = 0.0,
        .s_frames_left = 0,
        .s_frames_total = 0,
        .s_wave = osc_b_wave
    };

    sound_t sound_c = {
        .s_adsr = &adsr,
        .s_active = 0,
        .s_phase = 0.0,
        .s_frames_left = 0,
        .s_frames_total = 0,
        .s_wave = osc_c_wave
    };

    synth_2osc_t synth_2osc = {
        .osc_a = &sound,
        .osc_b = &sound_b
    };

    synth_3osc_t synth_3osc =  {
        .osc_a = &sound,
        .osc_b = &sound_b,
        .osc_c = &sound_c
    };

    snd_pcm_t *handle;
        
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        perror("snd_pcm_open");
        return 1;
    }

    int params_err = snd_pcm_set_params(handle, 
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        1, RATE, 1, 1000000);

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

    int running = 1;

    TTF_Init();
    TTF_Font* sans = TTF_OpenFont("FreeSans.ttf", 24);


    snd_rawmidi_t *midi_in;
    snd_rawmidi_open(&midi_in, NULL, "hw:0,0,0", SND_RAWMIDI_NONBLOCK);

    SDL_Event event;

    while (running) {

        while(SDL_PollEvent(&event) != 0) {
            if(event.type == SDL_QUIT) {
                running = 0;
            } 
        }

        unsigned char midi_buffer[1024];
        ssize_t ret = snd_rawmidi_read(midi_in, midi_buffer, sizeof(midi_buffer));
        if (ret > 0) {
            for (int i = 0; i + 2 < ret; i += 3) {
                unsigned char status = midi_buffer[i];
                unsigned char data1 = midi_buffer[i+1];
                unsigned char data2 = midi_buffer[i+2];

                if ((status & 0xF0) == 0x90 && data2 > 0) {
                    note.n_semitone = (data1 % 12);
                    note.n_octave = (data1 / 12) - 1;
                    note_to_sound(note, &sound);
                    note_to_sound(note, &sound_b);
                    sound_b.s_freq += 1;
                    note_to_sound(note, &sound_c);
                    sound_c.s_freq -= 1;
                }

                if ((status & 0xF0) == 0xB0) {
                    printf("Knob CC: %d Value: %d\n", data1, data2);
                    switch(data1) {
                        case 73:
                            adsr.att = (data2 / 127.0);
                            break;
                        case 75:
                            adsr.dec = (data2 / 127.0);
                            break;
                        case 79:
                            adsr.sus = (data2 / 127.0);
                            break;
                        case 72:
                            adsr.rel = (data2 / 127.0);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    

        render_synth3osc(synth_3osc, buffer);
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