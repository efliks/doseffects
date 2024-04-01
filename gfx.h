#ifndef _GFX_H
#define _GFX_H

void save_pal(void);
void renew_pal(void);
void init_pal(void);

void do_flash(void);
void fade_down(void);

void pixel(int x, int y, unsigned char color, unsigned char* where);

void draw_sprite(unsigned char* sprite, int x, int y, int width, int height, unsigned char* where);

void do_blur(unsigned char* frame_buffer, int width, int height);
void do_segment_blur(unsigned char* frame_buffer, int width);

#endif // _GFX_H
