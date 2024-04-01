/*
	3d Flag/Grid
	Mikolaj Felix 02/08/2001
	mfelix@polbox.com
*/

#include <time.h>

#include "low.h"
#include "clock.h"
#include "fontnew.h"
#include "math3d.h"
#include "scroll.h"

#define FLAG_WIDTH 16
#define FLAG_HEIGHT 16
#define FLAG_SIZE FLAG_WIDTH* FLAG_HEIGHT
#define CELL_SIZE 16
#define COS_STEP 20
#define MAX_FACES ((FLAG_WIDTH - 1) * (FLAG_HEIGHT - 1)) * 2
#define MAX_LINES MAX_FACES

#define MODE_FLAG 0
#define MODE_GRID 1

#define NO_DELAY 0
#define DELAY 1

#define SHIFT_CONST 6

typedef struct tFACE 
{
    int v1, v2, v3;
} FACE;

typedef struct tLINE 
{
    int v1, v2;
} LINE;

POINT3D flag_points[FLAG_SIZE];
POINT3D rotated_flag_points[FLAG_SIZE];
POINT2D translated_flag_points[FLAG_SIZE];
FACE faces[MAX_FACES];
LINE lines[MAX_LINES];

unsigned char colors[FLAG_SIZE];

int face_depth[MAX_FACES], order_table[MAX_FACES];

unsigned angle_x = 0;
unsigned angle_y = 0;
unsigned angle_z = 0;


//TODO Move to some common module
void draw_horizontal_line(int x1, int y, int x2, unsigned char color,
    unsigned char* buffer)
{
    int i, swap;
    unsigned char* ptr;

    //TODO Remove some checks?
    if (y >= 0 && y <= 319) {
        if (x1 > x2) {
            swap = x1;
            x1 = x2;
            x2 = swap;
        }

        if (x1 < 0) {
            x1 = 0;
        }
        if (x2 > 319) {
            x2 = 319;
        }

        ptr = buffer + y * 320 + x1;
        for (i = x1; i <= x2; i++, ptr++) {
            *ptr = color;
        }
    }
}

void draw_bresenham_line(int x1, int y1, int x2, int y2, unsigned char color,
    unsigned char* buffer)
{
    int dx, dy, max, x, y, d, dinc1, dinc2, xinc1, xinc2, yinc1, yinc2, i;

    dx = x2 - x1;
    if (dx < 0) {
        dx = -dx;
    }
    
    dy = y2 - y1;
    if (dy < 0) {
        dy = -dy;
    }

    if (dx >= dy) {
        max = dx + 1;
        d = (dy << 1) - dx;
        dinc1 = dy << 1;
        dinc2 = (dy - dx) << 1;
        xinc1 = 1;
        xinc2 = 1;
        yinc1 = 0;
        yinc2 = 1;
    } else {
        max = dy + 1;
        d = (dx << 1) - dy;
        dinc1 = dx << 1;
        dinc2 = (dx - dy) << 1;
        xinc1 = 0;
        xinc2 = 1;
        yinc1 = 1;
        yinc2 = 1;
    }

    if (x1 > x2) {
        xinc1 = -xinc1;
        xinc2 = -xinc2;
    }

    if (y1 > y2) {
        yinc1 = -yinc1;
        yinc2 = -yinc2;
    }

    x = x1;
    y = y1;

    for (i = 0; i < max; i++) {
        if (x >= 0 && x <= 319 && y >= 0 && y <= 199)
            buffer[x + (y << 6) + (y << 8)] = color;

        if (d < 0) {
            d += dinc1;
            x += xinc1;
            y += yinc1;
        } else {
            d += dinc2;
            x += xinc2;
            y += yinc2;
        }
    }
}

void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3,
    unsigned char color, unsigned char* buffer)
{
    int dx12, dx13, dx23, fixedx1, fixedx2, i;
    int line_x1, line_x2;
    int swap;

    if (y1 > y2) {
        swap = y1;
        y1 = y2;
        y2 = swap;

        swap = x1;
        x1 = x2;
        x2 = swap;
    }
    if (y2 > y3) {
        swap = y2;
        y2 = y3;
        y3 = swap;

        swap = x2;
        x2 = x3;
        x3 = swap;
    }
    if (y1 > y2) {
        swap = y1;
        y1 = y2;
        y2 = swap;

        swap = x1;
        x1 = x2;
        x2 = swap;
    }

    if ((i = y2 - y1) != 0)
        dx12 = ((x2 - x1) << SHIFT_CONST) / i;
    else
        dx12 = 0;

    if ((i = y3 - y1) != 0)
        dx13 = ((x3 - x1) << SHIFT_CONST) / i;
    else
        dx13 = 0;

    if ((i = y3 - y2) != 0)
        dx23 = ((x3 - x2) << SHIFT_CONST) / i;
    else
        dx23 = 0;

    fixedx1 = fixedx2 = x1 << SHIFT_CONST;
    for (i = y1; i < y2; i++) {
        if (i >= 0 && i <= 199) {
            line_x1 = fixedx1 >> SHIFT_CONST;
            line_x2 = fixedx2 >> SHIFT_CONST;

            if (line_x1 < 0)
                line_x1 = 0;
            else if (line_x1 > 319)
                line_x1 = 319;

            if (line_x2 < 0)
                line_x2 = 0;
            else if (line_x2 > 319)
                line_x2 = 319;

            draw_horizontal_line(line_x1, i, line_x2, color, buffer);
        }

        fixedx1 += dx13;
        fixedx2 += dx12;
    }

    fixedx2 = x2 << SHIFT_CONST;
    for (i = y2; i < y3; i++) {
        if (i >= 0 && i <= 199) {
            line_x1 = fixedx1 >> SHIFT_CONST;
            line_x2 = fixedx2 >> SHIFT_CONST;

            if (line_x1 < 0)
                line_x1 = 0;
            else if (line_x1 > 319)
                line_x1 = 319;

            if (line_x2 < 0)
                line_x2 = 0;
            else if (line_x2 > 319)
                line_x2 = 319;

            draw_horizontal_line(line_x1, i, line_x2, color, buffer);
        }

        fixedx1 += dx13;
        fixedx2 += dx23;
    }
}

void move_flag(int mode)
{
    static startoff;
    int i, j, cosoff, colortmp = 0;
    POINT3D* point;
    unsigned char *colorptr, add_col;

    point = &flag_points[0];
    colorptr = &colors[0];

    for (i = 0; i < FLAG_HEIGHT; i++) {
        for (j = 0; j < FLAG_WIDTH; j++) {
            cosoff = ((i + j + startoff) * COS_STEP) & (MAX_DEGS - 1);
            point->y = cosine[cosoff] * CELL_SIZE;

            if (mode == MODE_FLAG) {
                if ((++colortmp) & 1)
                    add_col = 0;
                else
                    add_col = 127;

                *colorptr = (unsigned char)((-cosine[cosoff] * 62) + 64 + 1 + add_col);
            } else {
                *colorptr = (unsigned char)((-cosine[cosoff] * 63) + 64 + 128);
            }

            point++;
            colorptr++;
        }
    }

    startoff++;
}

void init_lines(void)
{
    int i, j;
    LINE* line;

    line = &lines[0];

    for (i = 0; i < (FLAG_HEIGHT - 1); i++) {
        for (j = 0; j < (FLAG_WIDTH - 1); j++) {
            line->v1 = (i * FLAG_WIDTH) + j;
            line->v2 = (i * FLAG_WIDTH) + (j + 1);
            line++;

            line->v1 = (i * FLAG_WIDTH) + j;
            line->v2 = ((i + 1) * FLAG_WIDTH) + j;
            line++;
        }
    }
}

void init_flag(void)
{
    int i, j;
    POINT3D* point;

    point = &flag_points[0];

    for (i = 0; i < FLAG_HEIGHT; i++) {
        for (j = 0; j < FLAG_WIDTH; j++) {
            point->x = (FLAG_WIDTH * CELL_SIZE / 2) - (i * CELL_SIZE);
            point->z = (FLAG_HEIGHT * CELL_SIZE / 2) - (j * CELL_SIZE);

            point++;
        }
    }
}

void rotate_translate_flag(void)
{
    int i;
    POINT3D *srcpoint, *destpoint;
    POINT2D* point2d;

    srcpoint = &flag_points[0];
    destpoint = &rotated_flag_points[0];
    point2d = &translated_flag_points[0];

    for (i = 0; i < FLAG_SIZE; i++) {
        rotate_point(destpoint, srcpoint, angle_x, angle_y, angle_z);
        translate_point(point2d, destpoint);

        srcpoint++;
        destpoint++;
        point2d++;
    }
}

void flag_depth_sorting(int left, int right)
{
    int i, j, tmp, element;

    i = left;
    j = right;
    element = face_depth[(left + right) >> 1];

    while (i < j) {
        while (face_depth[i] < element)
            i++;
        while (face_depth[j] > element)
            j--;

        if (i <= j) {
            tmp = face_depth[i];
            face_depth[i] = face_depth[j];
            face_depth[j] = tmp;

            tmp = order_table[i];
            order_table[i] = order_table[j];
            order_table[j] = tmp;

            i++;
            j--;
        }
    }

    if (j > left)
        flag_depth_sorting(left, j);
    if (i < right)
        flag_depth_sorting(i, right);
}

void flag_sort_faces(void)
{
    int i, j;
    FACE* face;
    POINT3D* vertex;
    float depth;

    face = &faces[0];

    for (i = 0; i < MAX_FACES; i++) {
        order_table[i] = i;

        vertex = &rotated_flag_points[face->v1];
        depth = vertex->z;
        vertex = &rotated_flag_points[face->v2];
        depth += vertex->z;
        vertex = &rotated_flag_points[face->v3];
        depth += vertex->z;

        face_depth[i] = (int)(depth * 64);

        face++;
    }

    flag_depth_sorting(0, MAX_FACES - 1);
}

void draw_flag(int mode, unsigned char* frame_buffer)
{
    int i, x1, y1, x2, y2, x3, y3;
    unsigned char color;
    FACE* face;
    LINE* line;
    POINT2D* point;

    if (mode == MODE_FLAG) {
        flag_sort_faces();

        for (i = 0; i < MAX_FACES; i++) {
            face = &faces[order_table[MAX_FACES - 1 - i]];

            point = &translated_flag_points[face->v1];
            x1 = point->x;
            y1 = point->y;
            point = &translated_flag_points[face->v2];
            x2 = point->x;
            y2 = point->y;
            point = &translated_flag_points[face->v3];
            x3 = point->x;
            y3 = point->y;

            color = colors[face->v1];
            draw_triangle(x1, y1, x2, y2, x3, y3, color, frame_buffer);
        }
    } else {
        line = &lines[0];

        for (i = 0; i < MAX_LINES; i++) {
            point = &translated_flag_points[line->v1];
            x1 = point->x;
            y1 = point->y;
            point = &translated_flag_points[line->v2];
            x2 = point->x;
            y2 = point->y;

            color = colors[line->v1];
            draw_bresenham_line(x1, y1, x2, y2, color, frame_buffer);

            line++;
        }
    }
}

void init_palette_filled()
{
    int i;
    unsigned char my_pal[768], *ptr;
    
    dump_palette(my_pal);
    ptr = my_pal;
    
    *(ptr++) = 0;
    *(ptr++) = 0;
    *(ptr++) = 0;
    
    for (i = 0; i < 127; i++) {
        *(ptr++) = (i / 3) + 15;
        *(ptr++) = 0;
        *(ptr++) = 0;
    }
    for (i = 0; i < 127; i++) {
        *(ptr++) = (i / 3) + 15;
        *(ptr++) = (i / 3) + 15;
        *(ptr++) = (i / 3) + 15;
    }

    set_palette(my_pal);
}

void init_palette_wireframe()
{
    int i;
    unsigned char my_pal[768], *ptr;
    
    dump_palette(my_pal);
    ptr = my_pal + 3 * 128;
        
    for (i = 0; i < 128; i++) {
        *(ptr++) = (i / 3) + 16;
        *(ptr++) = (i / 3) + 16;
        *(ptr++) = (i / 3) + 16;
    }

    set_palette(my_pal);
}

void init_faces(void)
{
    int i, j;
    FACE* face;

    face = &faces[0];

    for (i = 0; i < (FLAG_HEIGHT - 1); i++) {
        for (j = 0; j < (FLAG_WIDTH - 1); j++) {
            face->v1 = (i * FLAG_WIDTH) + j;
            face->v2 = (i * FLAG_WIDTH) + (j + 1);
            face->v3 = ((i + 1) * FLAG_WIDTH) + (j + 1);
            face++;

            face->v1 = (i * FLAG_WIDTH) + j;
            face->v2 = ((i + 1) * FLAG_WIDTH) + (j + 1);
            face->v3 = ((i + 1) * FLAG_WIDTH) + j;
            face++;
        }
    }
}

void do_flag(unsigned char* frame_buffer)
{
    /*
    if (argc < 2)
        display_mode = DELAY;
    else {
        if (strncmp(argv[1], "-nd", 3) == 0)
            display_mode = NO_DELAY;
        else {
            printf("%s", "Incorrect argument!\n");
            exit(1);
        }
    }

    if ((frame_buffer = (unsigned char*)malloc(BUFFER_SIZE)) == NULL)
        exit(2);
    */
    
    time_t before;
    time_t after;
    
    unsigned long frames;
    float framerate;
    
    char txt_buffer[32];

    int mode = MODE_FLAG;
    int display_mode;

    init_flag();
    init_faces();
    init_lines();

    init_palette_filled();

    before = time(NULL);
    frames = 0;

    while (mode < 2) {
        timer_start(5);
        
        move_flag(mode);

        rotate_translate_flag();
        draw_flag(mode, frame_buffer);

        /*
        put_string("3d Flag/Grid", 0, 0, 240, frame_buffer);
        put_string("Mikolaj Felix 02/08/01", 0, 12, 240, frame_buffer);
        put_string("mfelix@polbox.com", 0, 24, 240, frame_buffer);

        frames++;
        after = time(NULL);

        if ((after - before)) {
            framerate = (float)(frames) / (float)(after - before);
            gcvt(framerate, 5, txt_buffer);
        } else
            txt_buffer[0] = 0;

        if (display_mode == DELAY) {
            retrace();
            retrace();
        } else {
            put_string("FPS: ", 0, 36, 240, frame_buffer);
            put_string(txt_buffer, 40, 36, 240, frame_buffer);
        }
        */
        do_scroll(199 - 8, frame_buffer);

        copy_buffer(frame_buffer);
        clear_buffer(frame_buffer);

        angle_x++;
        angle_x &= (MAX_DEGS - 1);
        angle_y++;
        angle_y &= (MAX_DEGS - 1);
        angle_z++;
        angle_z &= (MAX_DEGS - 1);

        timer_wait();

        if (is_key_pressed()) {
            get_key_code();
            mode++;
            init_palette_wireframe();
            frames = 0;
            before = time(NULL);
        }
    }
}
