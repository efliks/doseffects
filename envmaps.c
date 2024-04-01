/*
 * FlatGrd2 06/02/02
 * Mikolaj Felix a.k.a. Majuma
 * mfelix@polbox.com
 */

#include <math.h>
#include <stdlib.h>


unsigned char *envmap, *bumpmap;

int enable_envmap(void)
{
    float x, y, temp_color;
    unsigned char *ptr, color;

    envmap = (unsigned char*)malloc(128 * 128);
    if (!envmap) {
        return 0;
    }

    ptr = envmap;

    for (y = -64; y < 64; y++) {
        for (x = -64; x < 64; x++) {
            temp_color = (62 - sqrt(pow(x, 2) + pow(y, 2))) * 1.2;

            if (temp_color < 0)
                temp_color = 0;
            else if (temp_color > 62)
                temp_color = 62;

            color = (unsigned char)temp_color;
            *ptr = color;
            ptr++;
        }
    }

    return 1;
}

void disable_envmap(void)
{
    free(envmap);
}

int enable_bumpmap(void)
{
    int i, j;
    unsigned char* ptr;

    bumpmap = (unsigned char*)malloc(128 * 128);
    if (!bumpmap) {
        return 0;
    }

    // srand((int)rawclock());

    ptr = bumpmap;
    for (i = 0; i < 128 * 128; i++, ptr++)
        *ptr = rand() % 255;

    for (i = 0; i < 6; i++) {
        ptr = bumpmap;
        for (j = 0; j < 16384; j++) {
            *ptr = (bumpmap[(j - 1) & 16383] + bumpmap[(j + 1) & 16383] + bumpmap[(j - 128) & 16383] + bumpmap[(j + 128) & 16383]) >> 2;

            ptr++;
        }
    }

    return 1;
}

void disable_bumpmap(void)
{
    free(bumpmap);
}
