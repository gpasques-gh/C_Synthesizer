#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


#include "interface.h"
#include "synth.h"



void render_informations(params_t *params)
{

    // x start = 50

    // 50 + 20 = 70

    // 70 + 80 = 150
    // 600 = 150 + 450
    // 450 / 2 = 225

    /* ADSR envelope sliders */
    GuiGroupBox((Rectangle){ 30, 30, WIDTH / 2 - 50, 160 }, "ADSR Envelope");

    GuiLabel((Rectangle){ 155, 40, 100, 20}, "Attack");
    GuiSlider((Rectangle){ 60, 60, 225, 40 }, NULL, NULL, params->attack, 0.0f, 2.0f);

    GuiLabel((Rectangle){ 155, 110, 100, 20}, "Decay");
    GuiSlider((Rectangle){ 60, 130, 225, 40 }, NULL, NULL, params->decay, 0.0f, 2.0f);

    GuiLabel((Rectangle){ 415, 40, 100, 20}, "Sustain");
    GuiSlider((Rectangle){ 320, 60, 225, 40 }, NULL, NULL, params->sustain, 0.0f, 1.0f);

    GuiLabel((Rectangle){ 415, 110, 100, 20}, "Release");
    GuiSlider((Rectangle){ 320, 130, 225, 40 }, NULL, NULL, params->release, 0.0f, 1.0f);

    /* Filter ADSR envelope sliders */
    GuiGroupBox((Rectangle){ 625, 30, WIDTH / 2 - 50, 160 }, "Filter ADSR Envelope");

    GuiLabel((Rectangle){ 750, 40, 100, 20}, "Attack");
    GuiSlider((Rectangle){ 655, 60, 225, 40 }, NULL, NULL, params->synth->filter->adsr->attack, 0.0f, 2.0f);

    GuiLabel((Rectangle){ 750, 110, 100, 20}, "Decay");
    GuiSlider((Rectangle){ 655, 130, 225, 40 }, NULL, NULL, params->synth->filter->adsr->decay, 0.0f, 2.0f);

    GuiLabel((Rectangle){ 1010, 40, 100, 20}, "Sustain");
    GuiSlider((Rectangle){ 915, 60, 225, 40 }, NULL, NULL, params->synth->filter->adsr->sustain, 0.0f, 1.0f);

    GuiLabel((Rectangle){ 1010, 110, 100, 20}, "Release");
    GuiSlider((Rectangle){ 915, 130, 225, 40 }, NULL, NULL, params->synth->filter->adsr->release, 0.0f, 1.0f);

    /* Oscillators waveforms */
    GuiGroupBox((Rectangle){ 30, 220, WIDTH / 2 - 50, 160 }, "Oscillators");


    GuiLabel((Rectangle){100, 255, 100, 20}, "Oscillator A");
    if (GuiDropdownBox((Rectangle){60, 275, 140, 40 }, 
        "#01#Sine wave;#02#Square wave;#03#Triangle wave;#04#Sawtooth wave", 
        params->dropbox_a, *params->dropbox_a_b))
    {
        *params->dropbox_a_b = !*params->dropbox_a_b;
    }
    for (int v = 0; v < VOICES; v++)
        params->synth->voices[v].oscillators[0].wave = *params->dropbox_a;

    GuiLabel((Rectangle){270, 255, 100, 20}, "Oscillator B");
    if (GuiDropdownBox((Rectangle){230, 275, 140, 40 }, 
        "#01#Sine wave;#02#Square wave;#03#Triangle wave;#04#Sawtooth wave", 
        params->dropbox_b, *params->dropbox_b_b))
    {
        *params->dropbox_b_b = !*params->dropbox_b_b;
    }
    for (int v = 0; v < VOICES; v++)
            params->synth->voices[v].oscillators[1].wave = *params->dropbox_b;

            
    GuiLabel((Rectangle){440, 255, 100, 20}, "Oscillator C");
    if (GuiDropdownBox((Rectangle){400, 275, 140, 40 }, 
        "#01#Sine wave;#02#Square wave;#03#Triangle wave;#04#Sawtooth wave", 
        params->dropbox_c, *params->dropbox_c_b))
    {
        *params->dropbox_c_b = !*params->dropbox_c_b;
    }
    for (int v = 0; v < VOICES; v++)
            params->synth->voices[v].oscillators[2].wave = *params->dropbox_c;


    /* Synth parameters */
    GuiGroupBox((Rectangle){ 625, 220, WIDTH / 2 - 50, 160 }, "Synth parameters");

    GuiLabel((Rectangle){ 750, 230, 100, 20}, "Cutoff");
    GuiSlider((Rectangle){ 655, 250, 225, 40 }, NULL, NULL, &params->synth->filter->cutoff, 0.0f, 1.0f);

    GuiLabel((Rectangle){ 750, 300, 100, 20}, "Detune");
    GuiSlider((Rectangle){ 655, 320, 225, 40 }, NULL, NULL, &params->synth->detune, 0.0f, 1.0f);

    GuiLabel((Rectangle){ 1010, 230, 100, 20}, "Amp");
    GuiSlider((Rectangle){ 915, 250, 225, 40 }, NULL, NULL, &params->synth->amp, 0.0f, 1.0f);

    GuiCheckBox((Rectangle){915, 320, 40, 40}, "Filter envelope", &params->synth->filter->env);
}


/* Renders the waveform generated from the render_synth function into the SDL renderer */
void render_waveform(short *buffer)
{
    int mid_y = HEIGHT / 4;
    int y = HEIGHT / 3 + mid_y;

    int step = 1;

    for (int i = 0; i < FRAMES - step; i += step)
    {
        int x1 = (i * WIDTH) / FRAMES;
        int x2 = ((i + step) * WIDTH) / FRAMES;

        int y1 = y - ((buffer[i] * mid_y) / 32768);
        int y2 = y - ((buffer[i + step] * mid_y) / 32768);

        DrawLine(x1, y1, x2, y2, BLACK);
    }
}

/* Render the white keys from the MIDI piano visualizer into the SDL renderer */
void render_white_keys(SDL_Renderer *renderer)
{
    for (int i = 0; i < WHITE_KEYS; i++)
    {
        SDL_Rect key =
            {
                .h = WHITE_KEYS_HEIGHT,
                .w = WHITE_KEYS_WIDTH,
                .x = i * WHITE_KEYS_WIDTH,
                .y = 0};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &key);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &key);
    }
}

/* Render the black keys from the MIDI piano visualizer into the SDL renderer */
void render_black_keys(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_Rect background =
        {
            .h = WHITE_KEYS_HEIGHT,
            .w = WIDTH,
            .x = 0,
            .y = 0};

    SDL_RenderFillRect(renderer, &background);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    char black_keys_pattern[] = {1, 1, 0, 1, 1, 1, 0, 0};
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
                    .y = 0};
                SDL_RenderFillRect(renderer, &black_key);
            }
            white_key_index++;
            if (white_key_index >= WHITE_KEYS)
                break;
        }
        if (white_key_index >= WHITE_KEYS)
            break;
    }
}

/* Renders given note into a pressed key in the MIDI piano visualizer */
void render_key(SDL_Renderer *renderer, int midi_note)
{
    int width = 0, height = 0, x = 0, y = 0, is_black = 0;
    get_key_position(midi_note, &x, &y, &width, &height, &is_black);

    SDL_Rect key =
        {
            .h = height,
            .w = width,
            .x = x,
            .y = y};

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &key);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &key);
}

/* Outputs a given MIDI note rectangle parameters (x, y, width and height) */
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

/* Returns if a MIDI note is a assigned to a black key or not */
int is_black_key(int midi_note)
{
    int note = midi_note % 12;
    return (note == 1 || note == 3 || note == 6 || note == 8 || note == 10);
}