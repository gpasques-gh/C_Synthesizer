#ifndef DEFS_H
#define DEFS_H
#include <raylib.h>
#include <SDL2/SDL.h>

/* Notes semitones used for keyboard input */
#define nC 0
#define nC_SHARP 1
#define nD 2
#define nD_SHARP 3
#define nE 4
#define nF 5
#define nF_SHARP 6
#define nG 7
#define nG_SHARP 8
#define nA 9
#define nA_SHARP 10
#define nB 11

/* Keyboard layouts */
#define QWERTY 0
#define AZERTY 1

/* Keyboard note keys */
#define kC_QWERTY KEY_Q
#define kC_AZERTY KEY_A
#define kC_SHARP_QWERTY KEY_Z
#define kC_SHARP_AZERTY KEY_W
#define kD KEY_S
#define kD_SHARP KEY_E
#define kE KEY_D
#define kF KEY_F
#define kF_SHARP KEY_T
#define kG KEY_G
#define kG_SHARP KEY_Y
#define kA KEY_H
#define kA_SHARP KEY_U
#define kB KEY_J

/* MIDI packets informations */
#define MIDI_MAX_VALUE 127.0
#define PRESSED 0xF0
#define NOTE_ON 0x90
#define NOTE_OFF 0x80
#define KNOB_TURNED 0xB0

/* CC Values for the Arturia Keylab Essential 61 knobs */
/* ADSR parameters knobs */
#define ARTURIA_ATT_KNOB 1
#define ARTURIA_DEC_KNOB 2
#define ARTURIA_SUS_KNOB 3
#define ARTURIA_REL_KNOB 18

/* Oscillators waveforms knobs */
#define ARTURIA_OSC_A_KNOB 5
#define ARTURIA_OSC_B_KNOB 6
#define ARTURIA_OSC_C_KNOB 7

/* Synthesizer parameters knobs */
#define ARTURIA_CUTOFF_KNOB 8
#define ARTURIA_DETUNE_KNOB 4
#define ARTURIA_AMPLITUDE_KNOB 85

/* Note and synth related */
#define VOICES 6
#define DEFAULT_OCTAVE 4
#define A_4 440
#define RATE 44100
#define DEFAULT_AMPLITUDE 0.5
#define A4_POSITION 58

/* Oscillators waveforms */
#define SINE_WAVE 0
#define SQUARE_WAVE 1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3

/* ALSA buffering and latency */
#define FRAMES 512
#define LATENCY 40000
#define MAX_SAMPLES 512000

/* SDL interface */
#define WIDTH 1196
#define HEIGHT 800
#define TITLE "ALSA & SDL Synthesizer"

/* MIDI piano visualizer */
#define WHITE_KEYS 52
#define BLACK_KEYS 36
#define WHITE_KEYS_WIDTH WIDTH / WHITE_KEYS
#define WHITE_KEYS_HEIGHT HEIGHT / 4
#define BLACK_KEYS_WIDTH WHITE_KEYS_WIDTH / 2
#define BLACK_KEYS_HEIGHT (WHITE_KEYS_HEIGHT * 2) / 3

/* File parser states */
#define STATE_ADSR 0
#define STATE_IN_ADSR_NUMBER 1
#define STATE_FILTER_ADSR 2
#define STATE_IN_FILTER_NUMBER 3
#define STATE_SYNTH_PARAMETERS 4
#define STATE_IN_SYNTH_PARAMETERS_NUMBER 5
#define STATE_OSC_WAVEFORMS 6
#define STATE_IN_OSC_WAVEFORMS_NUMBER 7
#define STATE_ERROR 8
#define STATE_FINAL 9

#endif