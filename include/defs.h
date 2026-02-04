#ifndef DEFS_H
#define DEFS_H

#include <SDL2/SDL.h>

/**
 * Notes semitones used for keyboard input
 */
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

/**
 * Keyboard layouts
 */
#define QWERTY 0
#define AZERTY 1

/**
 * Keyboard note keys
 */
#define kC_QWERTY SDLK_a
#define kC_AZERTY SDLK_q
#define kC_SHARP_QWERTY SDLK_w
#define kC_SHARP_AZERTY SDLK_z
#define kD SDLK_s
#define kD_SHARP SDLK_e
#define kE SDLK_d
#define kF SDLK_f
#define kF_SHARP SDLK_t
#define kG SDLK_g
#define kG_SHARP SDLK_y
#define kA SDLK_h
#define kA_SHARP SDLK_u
#define kB SDLK_j

/**
 * Keyboard oscillators waveforms control keys
 */
#define OSC_A_WAVE_INCREMENT SDLK_1
#define OSC_B_WAVE_INCREMENT SDLK_2
#define OSC_C_WAVE_INCREMENT SDLK_3

/**
 * Synthesizer parameters control keys
 */
#define AMPLITUDE_INCREMENT SDLK_4
#define CUTOFF_INCREMENT SDLK_5
#define DETUNE_INCREMENT SDLK_6

/**
 * Keyboard ADSR envelope parameters control keys
 */
#define ATTACK_INCREMENT_QWERTY SDLK_z
#define ATTACK_INCREMENT_AZERTY SDLK_w
#define DECAY_INCREMENT SDLK_x
#define SUSTAIN_INCREMENT SDLK_c
#define RELEASE_INCREMENT SDLK_v

/**
 * MIDI packets informations
 */
#define MIDI_MAX_VALUE 127.0
#define PRESSED 0xF0
#define NOTE_ON 0x90
#define NOTE_OFF 0x80
#define KNOB_TURNED 0xB0

/**
 * CC Values for the Arturia Keylab Essential 61 knobs
 */
#define ARTURIA_ATT_KNOB 73
#define ARTURIA_DEC_KNOB 75
#define ARTURIA_SUS_KNOB 79
#define ARTURIA_REL_KNOB 72

#define ARTURIA_OSC_A_KNOB 80
#define ARTURIA_OSC_B_KNOB 81
#define ARTURIA_OSC_C_KNOB 82

#define ARTURIA_CUTOFF_KNOB 74
#define ARTURIA_DETUNE_KNOB 71
#define ARTURIA_AMPLITUDE_KNOB 85

/**
 * Note and synth related
 */
#define VOICES 6
#define DEFAULT_OCTAVE 4
#define A_4 440
#define A4_POSITION 58

/**
 * Oscillators waveforms
 */
#define SINE_WAVE 0
#define SQUARE_WAVE 1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3

/**
 * ALSA buffering and latency
 */
#define FRAMES 512
#define DEFAULT_AMPLITUDE 0.5
#define RATE 44100
#define LATENCY 40000

/**
 * SDL interface
 */
#define WIDTH 1200
#define HEIGHT 800
#define TITLE "ALSA & SDL Synthesizer"

/**
 * MIDI piano visualizer
 */
#define WHITE_KEYS 52
#define BLACK_KEYS 36
#define WHITE_KEYS_WIDTH WIDTH / WHITE_KEYS
#define WHITE_KEYS_HEIGHT HEIGHT / 4
#define BLACK_KEYS_WIDTH WHITE_KEYS_WIDTH / 2
#define BLACK_KEYS_HEIGHT (WHITE_KEYS_HEIGHT * 2) / 3

#endif