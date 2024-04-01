#ifndef _LOW_H
#define _LOW_H

void set_mode13h();
void unset_mode13h();

void retrace();

int is_key_pressed();
char get_key_code();

void copy_buffer(unsigned char *);
void clear_buffer(unsigned char *);

void set_palette(unsigned char *);
void dump_palette(unsigned char *);
void reset_palette();

#endif // _LOW_H
