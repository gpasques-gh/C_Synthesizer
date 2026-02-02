#ifndef DEFS_H
#define DEFS_H

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

// SOUND RELATED DEFS
#define FRAMES 512 // Length of the frame buffer
#define DEFAULT_AMPLITUDE 6000
#define MAX_AMPLITUDE DEFAULT_AMPLITUDE * 2
#define RATE 44100
#define LATENCY 30000
#define REVERB_BUFFER_SIZE (RATE * 2)

#endif 