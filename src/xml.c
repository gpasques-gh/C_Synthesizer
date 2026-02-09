#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#include "xml.h"

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
    char *preset_filename, bool *saving_preset)
{

    char filename[1024] = "presets/";

    /* Textbox for the preset name */
    int res = GuiTextInputBox((Rectangle){WIDTH / 2 - 100, HEIGHT / 2 - 50, 200, 100}, "Preset name :", "", "Save preset", preset_filename, 20, false);

    if (res == 0)
        /* If the textbox is closed */
        *saving_preset = false;
    /* If the textbox is submitted */
    else if (res == 1)
    {
        *saving_preset = false;
        strcat(filename, preset_filename);
        strcat(filename, ".xml");

        char text_element[1024];

        /* Getting the XML document pointer */
        xmlDocPtr doc = NULL;

        /* Getting the XML node pointers*/
        xmlNodePtr root_node = NULL;
        xmlNodePtr adsr_node = NULL;
        xmlNodePtr filter_node = NULL;
        xmlNodePtr filter_adsr_node = NULL;
        xmlNodePtr osc_node = NULL;
        xmlNodePtr effects_node = NULL;

        LIBXML_TEST_VERSION

        /* Initializing the XML document */
        doc = xmlNewDoc(BAD_CAST "1.0");

        /* Getting the root node */
        root_node = xmlNewNode(NULL, BAD_CAST "preset");
        xmlDocSetRootElement(doc, root_node);

        /* ADSR */
        adsr_node = xmlNewChild(root_node, NULL, BAD_CAST "adsr", NULL);
        /* Attack */
        snprintf(text_element, 1024, "%.2f", *attack);
        xmlNewChild(adsr_node, NULL, BAD_CAST "attack", BAD_CAST text_element);
        /* Decay */
        snprintf(text_element, 1024, "%.2f", *decay);
        xmlNewChild(adsr_node, NULL, BAD_CAST "decay", BAD_CAST text_element);
        /* Sustain */
        snprintf(text_element, 1024, "%.2f", *sustain);
        xmlNewChild(adsr_node, NULL, BAD_CAST "sustain", BAD_CAST text_element);
        /* Release */
        snprintf(text_element, 1024, "%.2f", *release);
        xmlNewChild(adsr_node, NULL, BAD_CAST "release", BAD_CAST text_element);

        /* Filter */
        filter_node = xmlNewChild(root_node, NULL, BAD_CAST "filter", NULL);
        /* Filter ADSR */
        filter_adsr_node = xmlNewChild(filter_node, NULL, BAD_CAST "filter_adsr", NULL);
        /* Attack */
        snprintf(text_element, 1024, "%.2f", *synth->filter->adsr->attack);
        xmlNewChild(filter_adsr_node, NULL, BAD_CAST "attack", BAD_CAST text_element);
        /* Decay */
        snprintf(text_element, 1024, "%.2f", *synth->filter->adsr->decay);
        xmlNewChild(filter_adsr_node, NULL, BAD_CAST "decay", BAD_CAST text_element);
        /* Sustain */
        snprintf(text_element, 1024, "%.2f", *synth->filter->adsr->sustain);
        xmlNewChild(filter_adsr_node, NULL, BAD_CAST "sustain", BAD_CAST text_element);
        /* Release */
        snprintf(text_element, 1024, "%.2f", *synth->filter->adsr->release);
        xmlNewChild(filter_adsr_node, NULL, BAD_CAST "release", BAD_CAST text_element);
        /* Filter cutoff */
        snprintf(text_element, 1024, "%.2f", synth->filter->cutoff);
        xmlNewChild(filter_node, NULL, BAD_CAST "cutoff", BAD_CAST text_element);
        /* Filter envelope ON/OFF */
        snprintf(text_element, 1024, "%d", synth->filter->env);
        xmlNewChild(filter_node, NULL, BAD_CAST "envelope_on", BAD_CAST text_element);

        /* Oscillators waveforms */
        osc_node = xmlNewChild(root_node, NULL, BAD_CAST "oscillators", NULL);
        /* Oscillator A */
        snprintf(text_element, 1024, "%d", *wave_a);
        xmlNewChild(osc_node, NULL, BAD_CAST "osc_a", BAD_CAST text_element);
        /* Oscillator B */
        snprintf(text_element, 1024, "%d", *wave_b);
        xmlNewChild(osc_node, NULL, BAD_CAST "osc_b", BAD_CAST text_element);
        /* Oscillator C */
        snprintf(text_element, 1024, "%d", *wave_c);
        xmlNewChild(osc_node, NULL, BAD_CAST "osc_c", BAD_CAST text_element);

        /* Effects */
        effects_node = xmlNewChild(root_node, NULL, BAD_CAST "effects", NULL);
        /* Detune */
        snprintf(text_element, 1024, "%.2f", synth->detune);
        xmlNewChild(effects_node, NULL, BAD_CAST "detune", BAD_CAST text_element);
        /* Amplification */
        snprintf(text_element, 1024, "%.2f", synth->amp);
        xmlNewChild(effects_node, NULL, BAD_CAST "amp", BAD_CAST text_element);

        /* Saving the XML document into the file */
        xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);
        xmlFreeDoc(doc);
        xmlCleanupParser();
    }

    return 0;
}

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
    int *wave_a, int *wave_b, int *wave_c)
{
    char filename[1024];
    FILE *f = popen("zenity --file-selection --filename '/home/germain/C_Synthesizer/presets/' --file-filter '*.xml'", "r");
    fgets(filename, 1024, f);
    filename[strcspn(filename, "\n")] = '\0';
    pclose(f);

    /* Getting the XML document pointer */
    xmlDoc *doc = NULL;

    /* Getting the XML node pointers*/
    xmlNode *root = NULL;
    xmlNode *node = NULL;

    /* Reading the XML file into the XML document pointer */
    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL)
    {
        fprintf(stderr, "failed to parse xml file.\n");
        return 1;
    }

    /* Getting the root node of the XML document */
    root = xmlDocGetRootElement(doc);

    /* Looping on the main nodes */
    for (node = root->children; node; node = node->next)
    {
        /* ADSR envelope */
        if (node->type == XML_ELEMENT_NODE &&
            xmlStrcmp(node->name, BAD_CAST "adsr") == 0)
        {
            parse_adsr(
                node, synth, attack,
                decay, sustain, release, false);
        }
        /* Filter */
        else if (node->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(node->name, BAD_CAST "filter") == 0)
        {
            xmlNode *child = NULL;
            /* Looping on the filter node children */
            for (child = node->children; child; child = child->next)
            { /* Filter ADSR envelope */
                if (child->type == XML_ELEMENT_NODE &&
                    xmlStrcmp(child->name, BAD_CAST "filter_adsr") == 0)
                {
                    parse_adsr(
                        child, synth, attack,
                        decay, sustain, release, true);
                }
                /* Filter cutoff */
                else if (child->type == XML_ELEMENT_NODE &&
                         xmlStrcmp(child->name, BAD_CAST "cutoff") == 0)
                {
                    xmlChar *cutoff = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    float cutoff_float = strtof((const char *)cutoff, &end_ptr);
                    if (end_ptr == (char *)cutoff)
                    {
                        fprintf(stderr, "bad cutoff value.\n");
                        return 1;
                    }
                    if (cutoff_float > 1.0)
                        cutoff_float = 1.0;
                    else if (cutoff_float < 0.0)
                        cutoff_float = 0.0;
                    synth->filter->cutoff = cutoff_float;
                }
                /* Filter ADSR envelope ON/OFF */
                else if (child->type == XML_ELEMENT_NODE &&
                         xmlStrcmp(child->name, BAD_CAST "envelope_on") == 0)
                {
                    xmlChar *envelope_on = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    int env_on_int = strtol((const char *)envelope_on, &end_ptr, 10);
                    if (end_ptr == (char *)envelope_on)
                    {
                        fprintf(stderr, "bad envelope value.\n");
                        return 1;
                    }
                    if (env_on_int > 1)
                        env_on_int = 1;
                    else if (env_on_int < 0)
                        env_on_int = 0;
                    synth->filter->env = env_on_int;
                }
            }
        }
        /* Oscillators waveforms */
        else if (node->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(node->name, BAD_CAST "oscillators") == 0)
        {
            xmlNode *child = NULL;
            /* Looping on the oscillators nodes*/
            for (child = node->children; child; child = child->next)
            { /* Oscillator A */
                if (child->type == XML_ELEMENT_NODE &&
                    xmlStrcmp(child->name, BAD_CAST "osc_a") == 0)
                {
                    xmlChar *osc_a = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    short osc_a_wave = strtol((const char *)osc_a, &end_ptr, 10);
                    if (end_ptr == (char *)osc_a)
                    {
                        fprintf(stderr, "bad osc a value.\n");
                        return 1;
                    }
                    if (osc_a_wave > 4)
                        osc_a_wave = 4;
                    else if (osc_a_wave < 0)
                        osc_a_wave = 0;
                    *wave_a = osc_a_wave;
                }
                /* Oscillator B */
                else if (child->type == XML_ELEMENT_NODE &&
                         xmlStrcmp(child->name, BAD_CAST "osc_b") == 0)
                {
                    xmlChar *osc_b = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    short osc_b_wave = strtol((const char *)osc_b, &end_ptr, 10);
                    if (end_ptr == (char *)osc_b)
                    {
                        fprintf(stderr, "bad osc b value.\n");
                        return 1;
                    }
                    if (osc_b_wave > 3)
                        osc_b_wave = 3;
                    else if (osc_b_wave < 0)
                        osc_b_wave = 0;
                    *wave_b = osc_b_wave;
                }
                /* Oscillator C */
                else if (child->type == XML_ELEMENT_NODE &&
                         xmlStrcmp(child->name, BAD_CAST "osc_c") == 0)
                {
                    xmlChar *osc_c = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    short osc_c_wave = strtol((const char *)osc_c, &end_ptr, 10);
                    if (end_ptr == (char *)osc_c)
                    {
                        fprintf(stderr, "bad osc b value.\n");
                        return 1;
                    }
                    if (osc_c_wave > 3)
                        osc_c_wave = 3;
                    else if (osc_c_wave < 0)
                        osc_c_wave = 0;
                    *wave_c = osc_c_wave;
                }
            }
        }
        /* Effects */
        else if (node->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(node->name, BAD_CAST "effects") == 0)
        {
            xmlNode *child = NULL;
            /* Looping on effects */
            for (child = node->children; child; child = child->next)
            {
                /* Detune effect */
                if (child->type == XML_ELEMENT_NODE &&
                    xmlStrcmp(child->name, BAD_CAST "detune") == 0)
                {
                    xmlChar *detune = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    float detune_float = strtof((const char *)detune, &end_ptr);
                    if (end_ptr == (char *)detune)
                    {
                        fprintf(stderr, "bad cutoff value.\n");
                        return 1;
                    }
                    if (detune_float > 1.0)
                        detune_float = 1.0;
                    else if (detune_float < 0.0)
                        detune_float = 0.0;
                    synth->detune = detune_float;
                }
                /* Amplification */
                else if (child->type == XML_ELEMENT_NODE &&
                         xmlStrcmp(child->name, BAD_CAST "amp") == 0)
                {
                    xmlChar *amp = xmlNodeGetContent(child);
                    char *end_ptr = NULL;
                    float amp_float = strtof((const char *)amp, &end_ptr);
                    if (end_ptr == (char *)amp)
                    {
                        fprintf(stderr, "bad cutoff value.\n");
                        return 1;
                    }
                    if (amp_float > 1.0)
                        amp_float = 1.0;
                    else if (amp_float < 0.0)
                        amp_float = 0.0;
                    synth->amp = amp_float;
                }
            }
        }
    }
    return 0;
}

/* Parse an ADSR XML Node whether it's basic ADSR of filter ADSR */
int parse_adsr(
    xmlNode *adsr_root_node,
    synth_t *synth,
    float *attack, float *decay,
    float *sustain, float *release,
    bool filter)
{
    xmlNode *child = NULL;
    /* Looping throught the child of the ADSR root node*/
    for (child = adsr_root_node->children; child; child = child->next)
    {
        /* Attack Node */
        if (child->type == XML_ELEMENT_NODE &&
            xmlStrcmp(child->name, BAD_CAST "attack") == 0)
        {
            xmlChar *attack_str = xmlNodeGetContent(child);
            char *end_ptr = NULL;
            float attack_float = strtof((const char *)attack_str, &end_ptr);
            if (end_ptr == (char *)attack_str)
            {
                fprintf(stderr, "bad attack value.\n");
                return 1;
            }
            if (attack_float > 2.0)
                attack_float = 2.0;
            else if (attack_float < 0.0)
                attack_float = 0.0;
            if (filter)
                *synth->filter->adsr->attack = attack_float;
            else
                *attack = attack_float;
        }
        /* Decay Node */
        else if (child->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(child->name, BAD_CAST "decay") == 0)
        {
            xmlChar *decay_str = xmlNodeGetContent(child);
            char *end_ptr = NULL;
            float decay_float = strtof((const char *)decay_str, &end_ptr);
            if (end_ptr == (char *)decay_str)
            {
                fprintf(stderr, "bad decay value.\n");
                return 1;
            }
            if (decay_float > 2.0)
                decay_float = 2.0;
            else if (decay_float < 0.0)
                decay_float = 0.0;
            if (filter)
                *synth->filter->adsr->decay = decay_float;
            else
                *decay = decay_float;
        }
        /* Sustain Node */
        else if (child->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(child->name, BAD_CAST "sustain") == 0)
        {
            xmlChar *sustain_str = xmlNodeGetContent(child);
            char *end_ptr = NULL;
            float sustain_float = strtof((const char *)sustain_str, &end_ptr);
            if (end_ptr == (char *)sustain_str)
            {
                fprintf(stderr, "bad sustain value.\n");
                return 1;
            }
            if (sustain_float > 2.0)
                sustain_float = 2.0;
            else if (sustain_float < 0.0)
                sustain_float = 0.0;
            if (filter)
                *synth->filter->adsr->sustain = sustain_float;
            else
                *sustain = sustain_float;
        }
        /* Release Node */
        else if (child->type == XML_ELEMENT_NODE &&
                 xmlStrcmp(child->name, BAD_CAST "release") == 0)
        {
            xmlChar *release_str = xmlNodeGetContent(child);
            char *end_ptr = NULL;
            float release_float = strtof((const char *)release_str, &end_ptr);
            if (end_ptr == (char *)release_str)
            {
                fprintf(stderr, "bad release value.\n");
                return 1;
            }
            if (release_float > 2.0)
                release_float = 2.0;
            else if (release_float < 0.0)
                release_float = 0.0;
            if (filter)
                *synth->filter->adsr->release = release_float;
            else
                *release = release_float;
        }
    }
    return 0;
}
