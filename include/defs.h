#ifndef DEFS_H
#define DEFS_H

#include <SDL2/SDL.h>

// NOTES SEMITONES
#define nC 1
#define nC_SHARP 2
#define nD 3
#define nD_SHARP 4
#define nE 5
#define nF 6
#define nF_SHARP 7
#define nG 8
#define nG_SHARP 9
#define nA 10
#define nA_SHARP 11
#define nB 12

// NOTE RELATED DEFS
#define DEFAULT_OCTAVE 4
#define A_4 440
#define A4_POSITION 58 // Position of A4 starting from C0

// KEYBOARD (AZERTY, change q to a and z to w for QWERTY)
#define kC SDLK_q
#define kC_SHARP SDLK_z
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

// SOUND RELATED DEFS
#define FRAMES 512 // Length of the frame buffer
#define AMPLITUDE 1500
#define RATE 44100

// SOUND WAVES
#define SINE_WAVE 0
#define SQUARE_WAVE 1
#define TRIANGLE_WAVE 2
#define SAWTOOTH_WAVE 3

// TEXT USER INTERFACE
#define WIDTH 600
#define HEIGHT 600
#define FILE_INTERFACE "interface/interface.txt"

#endif 