#include "defs.h"
#include "effects.h"

/* Applies an amount of distortion onto a sound buffer */
void distortion(short *buffer, float amount, bool overdriving)
{
    if (amount > 1.0)
        amount = 1.0;
    else if (amount < 0.0)
        amount = 0.0;

    short clip;
    
    if (overdriving)
        clip = (32767 / 2) *  (1 - amount);
    else 
        clip = 32767 *  (1 - amount);

    for (int i = 0; i < FRAMES; i++)
    {
        if (buffer[i] > clip)
            buffer[i] = clip;
        if (buffer[i] < -clip)
            buffer[i] = -clip;
    }
}