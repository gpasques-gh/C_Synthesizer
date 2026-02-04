#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "interface.h"
#include "synth.h"


static text_cache_t text_cache = {NULL, NULL, NULL, "", "", ""};

void render_infos(synth_t synth, 
    TTF_Font *font, SDL_Renderer *renderer,
    double attack, double decay, double sustain, double release)
{
    SDL_Color black = {0, 0, 0, 255};
    
    char title_buffer[256] = TITLE;

    if (strcmp(title_buffer, text_cache.last_title_text) != 0)
    {
        if (text_cache.title_texture)
            SDL_DestroyTexture(text_cache.title_texture);

        SDL_Surface *surface = TTF_RenderText_Solid(font, title_buffer, black);
        text_cache.title_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        strcpy(text_cache.last_title_text, title_buffer);
    }

    SDL_Rect title_rect = {.h=50, .w=WIDTH / 2, .x=WIDTH / 4, .y=0};
    SDL_RenderCopy(renderer, text_cache.title_texture, NULL, &title_rect);

    char envelope_buffer[256]; 
    snprintf(envelope_buffer, sizeof(envelope_buffer), 
        "Envelope - Attack: %.2f | Decay: %.2f | Sustain: %.2f | Release: %.2f", 
        attack, decay, sustain, release);

    if (strcmp(envelope_buffer, text_cache.last_envelope_text) != 0)
    {
        if (text_cache.envelope_texture)
            SDL_DestroyTexture(text_cache.envelope_texture);
        
        SDL_Surface *surface = TTF_RenderText_Solid(font, envelope_buffer, black);
        text_cache.envelope_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        strcpy(text_cache.last_envelope_text, envelope_buffer);
    }
    
    SDL_Rect envelope_rect = {.h = 50, .w = WIDTH * 0.95, .x = (WIDTH - WIDTH * 0.95) / 2, .y = 60};
    SDL_RenderCopy(renderer, text_cache.envelope_texture, NULL, &envelope_rect);
    
    char waveform_buffer[256];
    snprintf(waveform_buffer, sizeof(waveform_buffer), 
        "Waveforms - Osc A: %s | Osc B: %s | Osc C: %s", 
        get_wave_name(synth.voices[0].oscillators[0].wave), 
        get_wave_name(synth.voices[0].oscillators[1].wave), 
        get_wave_name(synth.voices[0].oscillators[2].wave));
    
    if (strcmp(waveform_buffer, text_cache.last_waveform_text) != 0)
    {
        if (text_cache.waveform_texture)
            SDL_DestroyTexture(text_cache.waveform_texture);
        
        SDL_Surface *surface = TTF_RenderText_Solid(font, waveform_buffer, black);
        text_cache.waveform_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        
        strcpy(text_cache.last_waveform_text, waveform_buffer);
    }
    
    SDL_Rect waveform_rect = {.h = 50, .w = WIDTH * 0.95, .x = (WIDTH - WIDTH * 0.95) / 2, .y = 120};
    SDL_RenderCopy(renderer, text_cache.waveform_texture, NULL, &waveform_rect);
}

void cleanup_text_cache()
{
    if (text_cache.envelope_texture)
        SDL_DestroyTexture(text_cache.envelope_texture);
    if (text_cache.waveform_texture)
        SDL_DestroyTexture(text_cache.waveform_texture);
    
    text_cache.envelope_texture = NULL;
    text_cache.waveform_texture = NULL;
}
void render_waveform(SDL_Renderer *renderer, short *buffer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    int mid_y = HEIGHT / 2;  
     
    // Dessiner seulement 1 point sur 8 ou 16 pour réduire le nombre d'appels
    int step = 8; // Ajustez selon vos besoins 
    
    for (int i = 0; i < FRAMES - step; i += step) 
    { 
        int x1 = (i * WIDTH) / FRAMES;
        int x2 = ((i + step) * WIDTH) / FRAMES;

        int y1 = mid_y - ((buffer[i] * mid_y) / 32768);
        int y2 = mid_y - ((buffer[i + step] * mid_y) / 32768);

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void render_keyboard_base(SDL_Renderer *renderer)
{
    char black_keys_pattern[] = {1, 1, 0, 1, 1, 1, 0, 0};

    for (int i = 0; i < WHITE_KEYS; i++)
    {
        SDL_Rect key = 
        {
            .h = WHITE_KEYS_HEIGHT,
            .w = WHITE_KEYS_WIDTH,
            .x = i * WHITE_KEYS_WIDTH,
            .y = 0
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &key);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &key);
    }


    int white_key_index = 0;
    for (int octave = 0; octave <= (WHITE_KEYS / 7); octave++)
    {
        for (int i = 0; i < 7; i++)
        {
            if (black_keys_pattern[i])
            {
                int x = (white_key_index * WHITE_KEYS_WIDTH) + WHITE_KEYS_WIDTH - (BLACK_KEYS_WIDTH / 2);
                SDL_Rect black_key = {
                    .h = BLACK_KEYS_HEIGHT,
                    .w = BLACK_KEYS_WIDTH,
                    .x = x,
                    .y = 0
                };
                SDL_RenderFillRect(renderer, &black_key);
            }
            white_key_index++;
            if (white_key_index >= WHITE_KEYS) break;
        }
        if (white_key_index >= WHITE_KEYS) break;
    }
}

void get_key_position(int midi_note, int *x, int *y, int *width, int *height, int *is_black)
{
    int note_in_octave = midi_note % 12;
    int octave = midi_note / 12;

    static const int black_keys[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
    *is_black = black_keys[note_in_octave];

    static const int white_key_map[] = {0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6};
    int white_key_in_octave = white_key_map[note_in_octave];

    int white_key_index = (octave * 7) + white_key_in_octave;
    
    if (*is_black)
    {
        *width = WHITE_KEYS_WIDTH / 2;
        *height = (WHITE_KEYS_HEIGHT * 2) / 3;
        *x = (white_key_index * WHITE_KEYS_WIDTH) + WHITE_KEYS_WIDTH - (*width / 2);
        *y = HEIGHT - WHITE_KEYS_HEIGHT;
    }
    else
    {
        *width = WHITE_KEYS_WIDTH;
        *height = WHITE_KEYS_HEIGHT;
        *x = white_key_index * WHITE_KEYS_WIDTH;
        *y = HEIGHT - WHITE_KEYS_HEIGHT;
    }
}


void render_key(SDL_Renderer *renderer, int midi_note) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    int width = 0, height = 0, x = 0, y = 0, is_black = 0;
    get_key_position(midi_note, &x, &y, &width, &height, &is_black);

    SDL_Rect key =
    {
        .h = height,
        .w = width,
        .x = x,
        .y = y
    };

    SDL_RenderFillRect(renderer, &key);
}   