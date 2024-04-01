/*
	Flat Polygons with Shadow
	Gouraud Shading (Moving Light)

	Mikolaj Felix 20/06/2001
	mfelix@polbox.com

	Texture Mapping & PCX support added on 13/07/2001
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "low.h"
#include "clock.h"
#include "math3d.h"
#include "pcx.h"
#include "polygon.h"

#define MAX_POINTS 8
#define MAX_FACES 6
#define SIZE 50

#define SHADOW_X -40
#define SHADOW_Y 40

#define FILLED 0
#define GOURAUD 1
#define TEXTURED 2

typedef struct tPOLYGON 
{
    int v1, v2, v3, v4;
    unsigned char color;
} POLYGON;

POINT3D object[MAX_POINTS] = { { -SIZE, -SIZE, SIZE }, { SIZE, -SIZE, SIZE }, { SIZE, SIZE, SIZE }, { -SIZE, SIZE, SIZE },
    { -SIZE, -SIZE, -SIZE }, { SIZE, -SIZE, -SIZE }, { SIZE, SIZE, -SIZE }, { -SIZE, SIZE, -SIZE } };

POLYGON polygons[MAX_FACES] = { { 0, 1, 2, 3, 45 }, { 5, 4, 7, 6, 40 }, { 1, 5, 6, 2, 80 }, { 4, 0, 3, 7, 14 },
    { 4, 5, 1, 0, 1 }, { 3, 2, 6, 7, 32 } };

POINT3D light, light_rotated;
POINT3D normal[MAX_FACES], normal_rotated[MAX_FACES];
POINT3D point_normal[MAX_POINTS], point_normal_rotated[MAX_POINTS];

POINT3D object_rotated[MAX_POINTS];
POINT2D object_translated[MAX_POINTS];

unsigned char *background;

unsigned char pol_texture[128 * 128];

// TODO Put all polygon code in one module
#define X1 0
#define Y1 1
#define X2 2
#define Y2 3

#define LEFT 0
#define RIGHT 1

static int polygon_edges[200][2];
static unsigned char polygon_colors[200][2];


void force_black_color_at_0(PCX* pcx)
{
    unsigned i, new_black_color;
    unsigned char b1, b2;

    for (i = 0; i < 768; i += 3)
        if (!pcx->palette[i] && !pcx->palette[i + 1] && !pcx->palette[i + 2])
            break;

    new_black_color = i;

    for (i = 0; i < 3; i++) {
        b1 = pcx->palette[i];
        b2 = pcx->palette[new_black_color + i];
        pcx->palette[i] = b2;
        pcx->palette[new_black_color + i] = b1;
    }

    for (i = 0; i < pcx->width * pcx->height; i++)
        if (!pcx->data[i])
            pcx->data[i] = new_black_color;
}

void generate_pol_texture(void)
{
    int x, y, off = 0;

    for (y = 0; y < 128; y++)
        for (x = 0; x < 128; x++)
            pol_texture[off++] = ((x ^ y) & 63) + 128 + 64;
}

void init_pol_texture(void)
{
    PCX pcx;

    if (load_pcx(&pcx, "TEXTURE.PCX", 1)) {
        if (pcx.width != 128 || pcx.height != 128) {
            unload_pcx(&pcx);
            generate_pol_texture();
        } else {
            memcpy(pol_texture, pcx.data, 128 * 128);
            force_black_color_at_0(&pcx);
            unload_pcx(&pcx);
            set_palette(pcx.palette);
        }
    } else {
        generate_pol_texture();
    }
}

void make_light_unitvector(float x1, float y1, float z1,
    float x2, float y2, float z2)
{
    float lenght, x, y, z;

    x = x2 - x1;
    y = y2 - y1;
    z = z2 - z1;

    lenght = sqrt(x * x + y * y + z * z);

    light.x = x / lenght;
    light.y = y / lenght;
    light.z = z / lenght;
}

void set_palette_light(void)
{
    int i;
    unsigned char my_pal[768], *ptr;

    dump_palette(my_pal);
    ptr = my_pal;

    *(ptr++) = 0;
    *(ptr++) = 0;
    *(ptr++) = 0;

    for (i = 1; i < 64 - 20; i++) {
        *(ptr++) = 20 + i;
        *(ptr++) = 20 + i;
        *(ptr++) = 0;
    }

    for (i = 0; i < 20; i++) {
        *(ptr++) = 63;
        *(ptr++) = 63;
        *(ptr++) = 63 - 20 + i;
    }
    
    set_palette(my_pal);
}

void set_palette2(void)
{
    int i;
    unsigned char my_pal[768], *ptr;

    dump_palette(my_pal);
    ptr = my_pal + 3 * 128;

    for (i = 0; i < 128; i++) {
        *(ptr++) = i >> 1;
        *(ptr++) = i >> 1;
        *(ptr++) = i >> 1;
    }
    
    set_palette(my_pal);
}

void gouraud_line(int x1, int y, int x2, unsigned char c1, 
    unsigned char c2, unsigned char* buffer)
{
    int delta_c, curr_c, i;
    int swap;
    unsigned char swapbyte;
    unsigned char* ofs;

    if (y < 0 || y > 199 || x1 == x2)
        return;

    if (x1 > x2) {
        swap = x1;
        x1 = x2;
        x2 = swap;

        swapbyte = c1;
        c1 = c2;
        c2 = swapbyte;
    }

    delta_c = ((c2 - c1) << 7) / (x2 - x1);
    curr_c = c1 << 7;

    ofs = buffer + y * 320 + x1;

    for (i = x1; i < x2; i++) {
        *ofs = (unsigned char)(curr_c >> 7);
        ofs++;
        curr_c += delta_c;
    }
}

//TODO Move to some common module
void draw_horizontal_line_dupl(int x1, int y, int x2, unsigned char color,
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

void horizontal_shadow_smooth_line(int x1, int x2, int y,
    unsigned char c1, unsigned char c2, unsigned char* buffer)
{
    int i, off1, off2, delta_c, cur_c;
    int swap;
    unsigned char b;

    if ((y + SHADOW_Y) < 0 || (y + SHADOW_Y) > 199) {
        gouraud_line(x1, y, x2, c1, c2, buffer);
        return;
    }

    if (x1 > x2) {
        swap = x1;
        x1 = x2;
        x2 = swap;
    }
    if (x1 == x2) {
        return;
    }

    delta_c = ((c2 - c1) << 7) / (x2 - x1);
    cur_c = c1 << 7;

    off1 = (y << 6) + (y << 8) + x1;
    off2 = ((y + SHADOW_Y) << 6) + ((y + SHADOW_Y) << 8);

    for (i = x1; i <= x2; i++) {
        if ((i + SHADOW_X) > 0 && (i + SHADOW_X) < 320) {
            b = buffer[off2 + i + SHADOW_X];

            if (b > 127) {
                b &= 63;
                buffer[off2 + i + SHADOW_X] = b + 128;
            }
        }

        buffer[off1++] = cur_c >> 7;
        cur_c += delta_c;
    }
}

void horizontal_shadow_line(int x1, int x2, int y, unsigned char color, 
    unsigned char* frame_buffer)
{
    int i, off1, off2;
    int swap;
    unsigned char b;

    if ((y + SHADOW_Y) < 0 || (y + SHADOW_Y) > 199) {
        draw_horizontal_line_dupl(x1, y, x2, color, frame_buffer);
        return;
    }

    if (x1 > x2) {
        swap = x1;
        x1 = x2;
        x2 = swap;
    }

    off1 = (y << 6) + (y << 8) + x1;
    off2 = ((y + SHADOW_Y) << 6) + ((y + SHADOW_Y) << 8);

    for (i = x1; i <= x2; i++) {
        if ((i + SHADOW_X) > 0 && (i + SHADOW_X) < 320) {
            b = frame_buffer[off2 + i + SHADOW_X];

            if (b > 127) {
                b &= 63;
                frame_buffer[off2 + i + SHADOW_X] = b + 128;
            }
        }

        frame_buffer[++off1] = color;
    }
}

void gouraud_scan_edge(int x1, int y1, int x2, int y2,
    unsigned char color1, unsigned char color2)
{
    int i, deltax, x, xfixed;
    int delta_c, fixed_c, color;
    int swap;
    unsigned char swap8;

    if (y1 == y2)
        return;

    if (y1 > y2) {
        swap = y1;
        y1 = y2;
        y2 = swap;

        swap = x1;
        x1 = x2;
        x2 = swap;

        swap8 = color1;
        color1 = color2;
        color2 = swap8;
    }

    deltax = ((x2 - x1) << 7) / (y2 - y1);
    xfixed = x1 << 7;

    delta_c = ((color2 - color1) << 7) / (y2 - y1);
    fixed_c = color1 << 7;

    for (i = y1; i < y2; i++) {
        x = xfixed >> 7;
        color = fixed_c >> 7;

        if (x < polygon_edges[i][LEFT]) {
            polygon_edges[i][LEFT] = x;
            polygon_colors[i][LEFT] = color;
        }

        if (x > polygon_edges[i][RIGHT]) {
            polygon_edges[i][RIGHT] = x;
            polygon_colors[i][RIGHT] = color;
        }

        xfixed += deltax;
        fixed_c += delta_c;
    }
}

void gouraud_polygon(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,
    unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4,
    unsigned char* buffer)
{
    int min_y, max_y, i;

    for (i = 0; i < 200; i++) {
        polygon_edges[i][LEFT] = 32000;
        polygon_edges[i][RIGHT] = -32000;
    }

    min_y = y1;
    if (y2 < min_y)
        min_y = y2;
    if (y3 < min_y)
        min_y = y3;
    if (y4 < min_y)
        min_y = y4;

    max_y = y1;
    if (y2 > max_y)
        max_y = y2;
    if (y3 > max_y)
        max_y = y3;
    if (y4 > max_y)
        max_y = y4;

    if (min_y < 0)
        min_y = 0;
    if (max_y > 199)
        max_y = 199;

    gouraud_scan_edge(x1, y1, x2, y2, c1, c2);
    gouraud_scan_edge(x2, y2, x3, y3, c2, c3);
    gouraud_scan_edge(x3, y3, x4, y4, c3, c4);
    gouraud_scan_edge(x4, y4, x1, y1, c4, c1);

    for (i = min_y; i < max_y; i++)
        /*gouraud_line(polygon_edges[i][LEFT],polygon_edges[i][RIGHT],i,
						polygon_colors[i][LEFT],polygon_colors[i][RIGHT]);*/

        horizontal_shadow_smooth_line(polygon_edges[i][LEFT], polygon_edges[i][RIGHT], i,
            polygon_colors[i][LEFT], polygon_colors[i][RIGHT], buffer);
}

void scan_edge(int x1, int y1, int x2, int y2)
{
    int i, deltax, x, xfixed;
    int swap;

    if (y1 == y2)
        return;

    if (y1 > y2) {
        swap = y1;
        y1 = y2;
        y2 = swap;

        swap = x1;
        x1 = x2;
        x2 = swap;
    }

    deltax = ((x2 - x1) << 7) / (y2 - y1);
    xfixed = x1 << 7;

    for (i = y1; i < y2; i++) {
        x = xfixed >> 7;

        if (x < polygon_edges[i][LEFT])
            polygon_edges[i][LEFT] = x;
        if (x > polygon_edges[i][RIGHT])
            polygon_edges[i][RIGHT] = x;

        xfixed += deltax;
    }
}

void polygon(int x1, int y1, int x2, int y2, int x3,
    int y3, int x4, int y4, unsigned char color, unsigned char* buffer)
{
    int min_y, max_y, i;

    for (i = 0; i < 200; i++) {
        polygon_edges[i][LEFT] = 32000;
        polygon_edges[i][RIGHT] = -32000;
    }

    min_y = y1;
    if (y2 < min_y)
        min_y = y2;
    if (y3 < min_y)
        min_y = y3;
    if (y4 < min_y)
        min_y = y4;

    max_y = y1;
    if (y2 > max_y)
        max_y = y2;
    if (y3 > max_y)
        max_y = y3;
    if (y4 > max_y)
        max_y = y4;

    if (min_y < 0)
        min_y = 0;
    if (max_y > 199)
        max_y = 199;

    scan_edge(x1, y1, x2, y2);
    scan_edge(x2, y2, x3, y3);
    scan_edge(x3, y3, x4, y4);
    scan_edge(x4, y4, x1, y1);

    for (i = min_y; i < max_y; i++)
        //horizontal_line(polygon_edges[i][LEFT],polygon_edges[i][RIGHT],i,color);
        horizontal_shadow_line(polygon_edges[i][LEFT], polygon_edges[i][RIGHT], i, color, buffer);
}

void init_obj_normals(void)
{
    int i;
    float x_normal, y_normal, z_normal, lenght;

    for (i = 0; i < MAX_FACES; i++) {

        /*
			Xnormal=(P2.Y-P1.Y)(P1.Z-P3.Z)-(P2.Z-P1.Z)(P1.Y-P3.Y)
			Ynormal=(P2.Z-P1.Z)(P1.X-P3.X)-(P2.X-P1.X)(P1.Z-P3.Z)
			Znormal=(P2.X-P1.X)(P1.Y-P3.Y)-(P2.Y-P1.Y)(P1.X-P3.X)
		*/

        x_normal = ((object[polygons[i].v2].y - object[polygons[i].v1].y) * (object[polygons[i].v1].z - object[polygons[i].v3].z)) - ((object[polygons[i].v2].z - object[polygons[i].v1].z) * (object[polygons[i].v1].y - object[polygons[i].v3].y));

        y_normal = ((object[polygons[i].v2].z - object[polygons[i].v1].z) * (object[polygons[i].v1].x - object[polygons[i].v3].x)) - ((object[polygons[i].v2].x - object[polygons[i].v1].x) * (object[polygons[i].v1].z - object[polygons[i].v3].z));

        z_normal = ((object[polygons[i].v2].x - object[polygons[i].v1].x) * (object[polygons[i].v1].y - object[polygons[i].v3].y)) - ((object[polygons[i].v2].y - object[polygons[i].v1].y) * (object[polygons[i].v1].x - object[polygons[i].v3].x));

        lenght = sqrt(x_normal * x_normal + y_normal * y_normal + z_normal * z_normal);

        normal[i].x = x_normal / lenght;
        normal[i].y = y_normal / lenght;
        normal[i].z = z_normal / lenght;
    }
}

void init_obj_point_normals(void)
{
    int i, j, which_face[8], num;
    float x, y, z, lenght;

    for (i = 0; i < MAX_POINTS; i++) {
        for (j = 0, num = 0; j < MAX_FACES; j++) {
            if (i == polygons[j].v1 || i == polygons[j].v2 || i == polygons[j].v3 || i == polygons[j].v4) {
                which_face[num] = j;
                num++;
            }
        }

        for (j = 0, x = 0, y = 0, z = 0; j < num; j++) {
            x += normal[which_face[j]].x;
            y += normal[which_face[j]].y;
            z += normal[which_face[j]].z;
        }

        x /= num;
        y /= num;
        z /= num;

        lenght = sqrt(x * x + y * y + z * z);

        point_normal[i].x = x / lenght;
        point_normal[i].y = y / lenght;
        point_normal[i].z = z / lenght;
    }
}

void draw_object(int mode, unsigned char* frame_buffer)
{
    int i, j;
    int x1, y1, x2, y2, x3, y3, x4, y4;
    float dot;
    unsigned char vcolors[4];
    int point_nr;
    int* vertexptr;

    for (i = 0; i < MAX_FACES; i++) {
        x1 = object_translated[polygons[i].v1].x;
        y1 = object_translated[polygons[i].v1].y;
        x2 = object_translated[polygons[i].v2].x;
        y2 = object_translated[polygons[i].v2].y;
        x3 = object_translated[polygons[i].v3].x;
        y3 = object_translated[polygons[i].v3].y;
        x4 = object_translated[polygons[i].v4].x;
        y4 = object_translated[polygons[i].v4].y;

        if (((y1 - y3) * (x2 - x1) - (x1 - x3) * (y2 - y1)) > 0) {

            switch (mode) {
            case FILLED:
                polygon(x1, y1, x2, y2, x3, y3, x4, y4, polygons[i].color, frame_buffer);
                break;
            case GOURAUD:

                vertexptr = &polygons[i].v1;
                for (j = 0; j < 4; j++) {
                    point_nr = *vertexptr;

                    dot = (point_normal_rotated[point_nr].x * light_rotated.x) + (point_normal_rotated[point_nr].y * light_rotated.y) + (point_normal_rotated[point_nr].z * light_rotated.z);

                    if (dot > 1 || dot < 0)
                        dot = 0;
                    vcolors[j] = (dot * 62) + 1;

                    vertexptr++;
                }

                gouraud_polygon(x1, y1, x2, y2, x3, y3, x4, y4,
                    vcolors[0], vcolors[1], vcolors[2], vcolors[3], frame_buffer);
                break;
            case TEXTURED:
                textured_polygon(x1, y1, x2, y2, x3, y3, x4, y4,
                    127, 0, 0, 0, 0, 127, 127, 127, pol_texture, frame_buffer);
            }
        }
    }
}

void translate_object(void)
{
    int i;

    for (i = 0; i < MAX_POINTS; i++)
        translate_point(&object_translated[i], &object_rotated[i]);
}

void rotate_object(unsigned angle_x, unsigned angle_y, unsigned angle_z)
{
    int i;

    angle_x &= (MAX_DEGS - 1);
    angle_y &= (MAX_DEGS - 1);
    angle_z &= (MAX_DEGS - 1);

    for (i = 0; i < MAX_POINTS; i++)
        rotate_point(&object_rotated[i], &object[i], angle_x, angle_y, angle_z);

    // rotate normals

    /*for(i=0;i<MAX_FACES;i++)
		rotate_point(&normal_rotated[i],&normal[i],angle_x,angle_y,angle_z);*/

    // rotate point-normals

    for (i = 0; i < MAX_POINTS; i++)
        rotate_point(&point_normal_rotated[i], &point_normal[i], angle_x, angle_y, angle_z);

    light_rotated.x = light.x;
    light_rotated.y = light.y;
    light_rotated.z = light.z;
}

void init_background(void)
{
    int i, j;
    unsigned char* ptr = background;

    for (i = 0; i < 200; i++) {
        for (j = 0; j < 320; j++, ptr++) {
            *ptr = 192 + (unsigned char)((j ^ i) & 63);
        }
    }
}

void recover_background(unsigned char* frame_buffer)
{
    memcpy(frame_buffer, background, 320 * 200);
}

void do_object3d(unsigned char* frame_buffer)
{
    int ax = 0, ay = 0, az = 0;
    int mode = 0;
    
    if ((background = (unsigned char *)malloc(64000)) == NULL) {
        return;
    }

    make_light_unitvector(0, 0, -100, 0, 0, 0);
    init_obj_normals();
    init_obj_point_normals();

    init_background();

    reset_palette();
    set_palette2();

    while (mode < 3) {
        timer_start(5);
        
        rotate_object(ax, ay, az);
        translate_object();
        draw_object(mode, frame_buffer);

        copy_buffer(frame_buffer);

        if (mode == FILLED || mode == GOURAUD)
            recover_background(frame_buffer);
        else
            clear_buffer(frame_buffer);

        ax++;
        ay += 3;
        az++;

        timer_wait();

        if (is_key_pressed()) {
            get_key_code();
            mode++;

            if (mode == GOURAUD)
                set_palette_light();
            else if (mode == TEXTURED)
                init_pol_texture();
        }
    }

    free(background);
}
