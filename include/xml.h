#ifndef XML_H
#define XML_H

#include <raygui.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#include "synth.h"

/*
 * Saving a preset into an XML file :
 * - ADSR envelope parameters
 * - Filter ADSR envelope parameters and ON/OFF
 * - Filter cutoff
 * - Oscillators waveforms
 * - Detune effect
 * - Amplification
 */
int save_preset(
    synth_t *synth,
    float *attack, float *decay,
    float *sustain, float *release,
    int *wave_a, int *wave_b, int *wave_c,
    char *preset_filename, bool *saving_preset);

/*
 * Load a preset from an XML file to the application :
 * - ADSR envelope parameters
 * - Filter ADSR envelope parameters and ON/OFF
 * - Filter cutoff
 * - Oscillators waveforms
 * - Detune effect
 * - Amplification
 */
int load_preset(
    synth_t *synth,
    float *attack, float *decay,
    float *sustain, float *release,
    int *wave_a, int *wave_b, int *wave_c);

/* Parse an ADSR XML Node whether it's basic ADSR of filter ADSR */
int parse_adsr(
    xmlNode *adsr_root_node,
    synth_t *synth,
    float *attack, float *decay,
    float *sustain, float *release,
    bool filter);

#endif