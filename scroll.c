/*****************************************
  NAAGTRO version 2

  scroll.c   Scrolling text effect

  (C) March 5, 2023  M. Feliks
*****************************************/

#include <string.h>

#include "scroll.h"
#include "fontnew.h"

static char scrolltext[] = "Hi!!!!! I combined my old graphical effects into one package. Finally! These are only the effects implemented in C. The effects implemented in Assembler are a different story... To switch to the next effect, press any key. They don't switch automatically. Once you reach the last effect, this demo will end. Enjoy!                              ";

static unsigned char scrollbuffer[320 * 8];
static unsigned int curr_char = 0, curr_col = 0;


void init_scroll()
{
    memset(scrollbuffer, 0, 320 * 8);
}

void do_scroll(int y, unsigned char* frame_buffer)
{
    char my_char;
    unsigned char *ptr_char, *ptr_buffer, *ptr_fb;

    int col_idx;
    int i, j;

    my_char = scrolltext[curr_char % ( (sizeof (scrolltext) / sizeof (char)) - 1)];
    if (my_char >= 'a' && my_char <= 'z') {
       my_char += ('A' - 'a');
    }

    ptr_char = &chardata[((my_char - ' ') & 63) << 3];
    col_idx = 7 - (curr_col & 7);

    for (i = 0; i < 8; i++, ptr_char++) {
        if ((*ptr_char >> col_idx) & 1) {
            *(scrollbuffer + i * 320 + 319) = (unsigned char)(64 + i);
        }
        else {
            *(scrollbuffer + i * 320 + 319) = 0;
        }
    }

    for (i = 0; i < 8; i++) {
        ptr_buffer = scrollbuffer + 320 * i;
        for (j = 0; j < 319; j++, ptr_buffer++) {
            *ptr_buffer = *(ptr_buffer + 1);
        }
    }

    ptr_buffer = scrollbuffer;
    ptr_fb = frame_buffer + y * 320;

    for (i = 0; i < 320 * 8; i++, ptr_buffer++, ptr_fb++) {
        if (*ptr_buffer != 0) {
           *ptr_fb = *ptr_buffer;
        }
    }

    curr_col++;
    if (curr_col % 8 == 0) {
       curr_char++;
    }
}
