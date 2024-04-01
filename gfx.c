#include <string.h>
#include "low.h"

static unsigned char palette[768];

void pixel(int x, int y, unsigned char color, unsigned char* where)
{
    if (x >= 0 && x <= 319 && y >= 0 && y <= 199) {
        *(where + x + y * 320) = color;
    }
}

void draw_sprite(unsigned char* sprite, int x, int y, int width, int height, unsigned char* where)
{
    int i, j;
    
    unsigned char* sprptr = sprite;
    unsigned char* bptr = where + x + y * 320;

    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++, sprptr++, bptr++) {
            if (*sprptr != 0) {
                *bptr = *sprptr;
            }
        }
        bptr += 320 - width;
    }
}

void save_pal(void)
{
    dump_palette(palette);
}

void renew_pal(void)
{
    set_palette(palette);
}

void init_pal(void)
{
    int i;
    unsigned char my_pal[768], *ptr;

    dump_palette(my_pal);
    ptr = my_pal + 100 * 3;

    for (i = 0; i < 64; i++) {
        *(ptr++) = 0;
        *(ptr++) = 0;
        *(ptr++) = i;
    }
    for (i = 0; i < 64; i++) {
        *(ptr++) = i;
        *(ptr++) = 0;
        *(ptr++) = 0;
    }
    
    set_palette(my_pal);
}

void do_flash(void)
{
    int i, j;
    unsigned char my_pal[768], dest_pal[768], *srcp, *destp;

    dump_palette(dest_pal);

    memset(my_pal, 63, 768);
    set_palette(my_pal);

    for (i = 0; i < 64; i++) {
        srcp = my_pal;
        destp = dest_pal;

        for (j = 0; j < 768; j++) {
            if ((*srcp) > (*destp)) {
               (*srcp)--;
            }
            srcp++;
            destp++;
        }
        
        set_palette(my_pal);
        retrace();
    }
}

void fade_down(void)
{
    int i, j;
    unsigned char my_pal[768], *srcp;
    
    dump_palette(my_pal);

    for (j = 0; j < 64; j++) {
        
        srcp = my_pal;
        for (j = 0; j < 768; j++) {
            if ((*srcp) > 0) {
               (*srcp)--;
            }
            srcp++;
        }
        
        set_palette(my_pal);
        retrace();
    }
}

void do_blur(unsigned char* frame_buffer, int width, int height)
{
    int i;
    int color;
    unsigned char* pbf = frame_buffer;

    for (i = 0; i < width; i++) {
        *pbf = 0;
        pbf++;
    }

    for (i = 0; i < width * (height - 2); i++) {
        color = *(pbf - 1);
        color += *(pbf + 1);
        color += *(pbf - width);
        color += *(pbf + width);
        color >>= 2;
        *pbf = (unsigned char)color;
        pbf++;
    }

    for (i = 0; i < width; i++) {
        *pbf = 0;
        pbf++;
    }
}

void do_segment_blur(unsigned char* frame_buffer, int width)
{
    int i;
    int color;

    for (i = 0; i <= 0xffff; i++) {
        color = *(frame_buffer + ((i - 1) & 0xffff));
        color += *(frame_buffer + ((i + 1) & 0xffff));
        color += *(frame_buffer + ((i - width) & 0xffff));
        color += *(frame_buffer + ((i + width) & 0xffff));
        color >>= 2;
        *(frame_buffer + i) = (unsigned char)color;
    }
}
