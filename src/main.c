#ifndef RAYGUI_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION
#endif 

#include <unistd.h>
#include <alsa/asoundlib.h>


#include "defs.h"
#include "interface.h"
#include "synth.h"
#include "midi.h"
#include "keyboard.h"
#include "record.h"
#include "effects.h"

/* Prints the usage of the CLI arguments into the error output */
void usage()
{
    fprintf(stderr, "synth -kb : keyboard input, defaults to QWERTY\n");
    fprintf(stderr, "synth -kb <QWERTY/AZERTY>: keyboard input with the given keyboard layout\n");
    fprintf(stderr, "synth -midi <midi hardware id> : midi keyboard input, able to change parameters of the sounds (ADSR, cutoff, detune and oscillators waveforms)\n");
    fprintf(stderr, "use amidi -l to list your connected midi devices and find your midi device hardware id, often something like : hw:0,0,0 or hw:1,0,0\n");
    fprintf(stderr, "to see this helper again, use synth -h or synth -help\n");
}

/* Main function */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        usage();
        return 1;
    }

    /* CLI arguments variables */
    char midi_device[256];
    int midi_input = 0;
    int keyboard_input = 0;
    int keyboard_layout = QWERTY;

    /* CLI arguments handling */
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

    /* Oscillators waveforms */
    int osc_a = SINE_WAVE, osc_b = SINE_WAVE, osc_c = SINE_WAVE;

    /* Synthesizer ADSR envelope parameters*/
    float attack = 0.2;
    float decay = 0.3;
    float sustain = 0.7;
    float release = 0.2;

    /* Filter ADSR envelope parameters*/
    float filter_attack = 0.0;
    float filter_decay = 0.3;
    float filter_sustain = 0.0;
    float filter_release = 0.2;

    /* Initialize the filter ADSR envelope */
    adsr_t filter_adsr =
        {
            .attack = &filter_attack,
            .decay = &filter_decay,
            .sustain = &filter_sustain,
            .release = &filter_release};

    /* Initialize the filter */
    lp_filter_t filter =
        {
            .cutoff = 0.5,
            .prev_input = 0.0,
            .prev_output = 0.0,
            .adsr = &filter_adsr,
            .env = false};

    /* Initialize the synthesizer */
    synth_t synth =
        {
            .voices = malloc(sizeof(voice_t) * VOICES),
            .amp = DEFAULT_AMPLITUDE,
            .detune = 0.0,
            .filter = &filter};

    /* Error while allocating the synthesizer voices */
    if (synth.voices == NULL)
    {
        fprintf(stderr, "memory allocation failed.\n");
        return 1;
    }

    /* Create the synth voices */
    for (int i = 0; i < VOICES; i++)
    {
        /* Allocate voices */
        synth.voices[i].adsr = malloc(sizeof(adsr_t));
        if (synth.voices[i].adsr == NULL)
        {   /* Allocation error */
            fprintf(stderr, "memory allocation failed.\n");
            for (int j = 0; j < i; j++)
            {
                free(synth.voices[j].adsr);
                free(synth.voices[j].oscillators);
            }
            free(synth.voices);
            return 1;
        }

        /* ADSR envelope */
        synth.voices[i].adsr->attack = &attack;
        synth.voices[i].adsr->decay = &decay;
        synth.voices[i].adsr->sustain = &sustain;
        synth.voices[i].adsr->release = &release;
        synth.voices[i].adsr->state = ENV_IDLE;
        synth.voices[i].adsr->output = 0.0;

        /* Note and activation */
        synth.voices[i].note = 0;
        synth.voices[i].velocity_amp = 0.0;
        synth.voices[i].active = 0;

        /* Allocate oscillators */
        synth.voices[i].oscillators = malloc(sizeof(osc_t) * 3);
        if (synth.voices[i].oscillators == NULL)
        {   /* Allocation error*/
            fprintf(stderr, "memory allocation failed.\n");
            goto cleanup_synth;
        }

        /* Initializing oscillators*/
        for (int j = 0; j < 3; j++)
        {
            synth.voices[i].oscillators[j].freq = 0.0;
            synth.voices[i].oscillators[j].phase = 0.0;
        }

        /* Initializing oscillators wave pointers */
        synth.voices[i].oscillators[0].wave = &osc_a;
        synth.voices[i].oscillators[1].wave = &osc_b;
        synth.voices[i].oscillators[2].wave = &osc_c;
    }

    /* Open sound card */
    snd_pcm_t *handle = NULL;
    if (snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        fprintf(stderr, "error while opening sound card.\n");
        goto cleanup_synth;
    }

    /* Initialize sound card parameters */
    int params_err = snd_pcm_set_params(
        handle,
        SND_PCM_FORMAT_S16_LE,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        MONO, RATE, 1, LATENCY);

    if (params_err < 0)
    {
        fprintf(stderr, "error while setting sound card parameters: %s\n", snd_strerror(params_err));
        goto cleanup_alsa;
    }

    snd_pcm_prepare(handle);

    /* Sound buffer */
    short buffer[FRAMES];

    /* We send one sound buffer of silence
    into the sound card to avoid noise at start */
    memset(buffer, 0, sizeof(short) * FRAMES);
    snd_pcm_writei(handle, buffer, FRAMES);

    /* Initialize MIDI input */
    snd_rawmidi_t *midi_in;
    if (midi_input)
        if (snd_rawmidi_open(&midi_in, NULL, midi_device, SND_RAWMIDI_NONBLOCK) < 0)
        {
            fprintf(stderr, "error while opening midi device %s\n", midi_device);
            goto cleanup_alsa;
        }

    /* WAV file related variables */
    char audio_filename[1024] = "\0";
    FILE *fwav = NULL;
    wav_header_t header;
    unsigned int count = 0;
    bool recording = false;

    /* GUI rendering and interacting related variables */
    /* Dropdown menus booleans */
    bool ddm_a = false, ddm_b = false, ddm_c = false;
    /* Saving name booleans to avoid triggering notes with keyboard */
    bool saving_preset = false, saving_audio_file = false;
    char preset_filename[1024] = "\0";

    /* Initialize raylib window and font */
    InitWindow(WIDTH, HEIGHT, "ALSA & raygui synthesizer");
    Font annotation = LoadFont("Regular.ttf");
    GuiSetFont(annotation);
    GuiSetStyle(DEFAULT, TEXT_SIZE, GuiGetFont().baseSize * 0.5);

    /* Main loop */
    while (!WindowShouldClose())
    {
        /* Getting keyboard input if we are not currently inputing a preset or an audio file name*/
        if (keyboard_input && !saving_preset && !saving_audio_file)
        {
            /* Handling the pressed keys of the keyboard layout */
            handle_input(&synth, keyboard_layout, &octave);
            /* Handling the released keys of the keyboard layout */
            handle_release(&synth, keyboard_input, octave);
        }

        /* If we are using MIDI, get the MIDI input */
        if (midi_input)
            get_midi(midi_in, &synth, &attack, &decay, &sustain, &release);

        /* Render the synth into the sound buffer*/
        render_synth(&synth, buffer);
        distortion(buffer, 0.6, true);

        /* Write the buffer to the sound card */
        int err = snd_pcm_writei(handle, buffer, FRAMES);
        if (err == -EPIPE)
        {   /* If the application underruned*/
            fprintf(stderr, "ALSA underrun!\n");
            snd_pcm_prepare(handle);
        }
        else if (err < 0)
        {   /* Error during the writing of the buffer into the sound card */
            fprintf(stderr, "ALSA write error: %s\n", snd_strerror(err));
            snd_pcm_prepare(handle);
        }

        /* WAV file management */
        if (fwav != NULL && recording == true)
        {   /* The recording is ongoing and the file has already been created,
            so we write the sound buffer to the WAV file and increment the file writing counter */
            fwrite(buffer, 2, FRAMES, fwav);
            count++;
        }
        else if (fwav == NULL && recording == true)
        {   /* The user started the recording from the GUI and inserted the audio filename,
            so we initialize the WAV header and create the wav file with the given filename */
            char audio_full_filename[1024] = "audio/";
            strcat(audio_full_filename, audio_filename);
            strcat(audio_full_filename, ".wav");
            init_wav_header(&header);
            init_wav_file(audio_full_filename, &fwav, &header);
            audio_filename[0] = '\0';
        }
        else if (fwav != NULL && recording == false)
        {   /* The user stopped the recording from the GUI,
            so we are changing the WAV header to have the correct byte size for the data field.
            We then close the file and reset the writing counter to zero. */
            header.sub2_size = FRAMES * count * (unsigned int)header.num_channels * (unsigned int)header.bits_per_sample / 8;
            header.chunk_size = (unsigned int)header.sub2_size + 36;
            fseek(fwav, 0, SEEK_SET);
            fwrite(&header, 1, sizeof(header), fwav);
            close_wav_file(fwav);
            fwav = NULL;
            count = 0;
        }

        /* Drawing the GUI */
        BeginDrawing();
            /* Clearing the background */
            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
            /* Rendering the waveform visualizer */
            render_waveform(buffer);
            /* Rendering the informations GUI */
            render_informations(
                &synth,
                &attack, &decay, &sustain, &release,
                &osc_a, &osc_b, &osc_c,
                &ddm_a, &ddm_b, &ddm_c,
                preset_filename, audio_filename,
                &saving_preset, &saving_audio_file, &recording);
            /* We render the white keys and the pressed white keys before the black keys
            so that the black keys correctly overlap with the white keys */
            render_white_keys();
            for (int v = 0; v < VOICES; v++)
                if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && !is_black_key(synth.voices[v].note))
                    render_key(synth.voices[v].note);
            /* Now we render the black keys */
            render_black_keys();
            for (int v = 0; v < VOICES; v++)
                if (synth.voices[v].active && synth.voices[v].adsr->state != ENV_RELEASE && is_black_key(synth.voices[v].note))
                    render_key(synth.voices[v].note);
        EndDrawing();
    }

    /* We are closing the raylib window*/
    CloseWindow();

    /* We are closing the MIDI input*/
    if (midi_in)
        snd_rawmidi_close(midi_in);

    /* If we quit the application during recording, change WAV header and close WAV file */
    if (fwav != NULL && recording)
    {
        header.sub2_size = FRAMES * count * (unsigned int)header.num_channels * (unsigned int)header.bits_per_sample / 8;
        header.chunk_size = (unsigned int)header.sub2_size + 36;
        fseek(fwav, 0, SEEK_SET);
        fwrite(&header, 1, sizeof(header), fwav);
        close_wav_file(fwav);
    }

/* Cleanup gotos */
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
