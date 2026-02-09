#include "record.h"
#include "defs.h"

/* Initialize wav header */
int init_wav_header(wav_header_t *header)
{
    /* RIFF init*/
    header->chunk_id[0] = 'R';
    header->chunk_id[1] = 'I';
    header->chunk_id[2] = 'F';
    header->chunk_id[3] = 'F';
    /* WAVE init */
    header->format[0] = 'W';
    header->format[1] = 'A';
    header->format[2] = 'V';
    header->format[3] = 'E';
    /* fmt init */
    header->sub1_id[0] = 'f';
    header->sub1_id[1] = 'm';
    header->sub1_id[2] = 't';
    header->sub1_id[3] = ' ';
    /* data init */
    header->sub2_id[0] = 'd';
    header->sub2_id[1] = 'a';
    header->sub2_id[2] = 't';
    header->sub2_id[3] = 'a';

    header->num_channels = 1; /* 1 channel for mono, 2 for stereo */
    header->bits_per_sample = 16;
    /* Maximum data size for long recording, the size is updated when the recording is finished */
    header->sub2_size = 300 * MAX_SAMPLES * (unsigned int) header->num_channels * (unsigned int) header->bits_per_sample / 8;
    header->chunk_size = (unsigned int) header->sub2_size + 36;
    header->sub1_size = 16;
    header->audio_format = 1;
    header->sample_rate = RATE;
    header->byte_rate = 
        (unsigned int) header->sample_rate *
        (unsigned int) header->num_channels *
        (unsigned int) header->bits_per_sample / 8;
    header->block_align = (unsigned int) header->num_channels * (unsigned int) header->bits_per_sample / 8;
        
    return 0;
}

/* Initialize a wav file with a filename and wav header */
int init_wav_file(char *fname, FILE **fwav, wav_header_t *header)
{
    *fwav = fopen(fname, "wb");

    if (*fwav != NULL)
    {
        fwrite(header, 1, sizeof(*header), *fwav);
    }
    else
    {
        fprintf(stderr, "cannot open wav file to write data\n");
        return 1;
    }
    return 0;
}

/* Close a wav file */
int close_wav_file(FILE *fwav)
{
    if (fwav != NULL)
        fclose(fwav);
    else 
    {
        fprintf(stderr, "cannot close wav file\n");
        return 1;
    }
    return 0;
}
