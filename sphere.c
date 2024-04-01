/*
	Filled Sphere
	Mikolaj Felix 01/08/2001
	mfelix@polbox.com
*/

#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "low.h"
#include "clock.h"
#include "fontnew.h"
#include "math3d.h"
#include "triangle.h"
#include "scroll.h"

#define POINTS_PER_CIRCLE 20
#define SPHERE_RADIUS 90
#define MAX_POINTS POINTS_PER_CIRCLE* POINTS_PER_CIRCLE
#define ANGLE_STEP (float)(M_PI / (180 / POINTS_PER_CIRCLE))
#define MAX_FACES ((POINTS_PER_CIRCLE - 1) * (POINTS_PER_CIRCLE - 1)) * 2

#define TEXT_COLOR 64

#define VECTOR POINT3D

typedef struct tFACE 
{
    int v1, v2, v3;
} FACE;

POINT3D sphere[MAX_POINTS];
POINT3D rotated_sphere[MAX_POINTS];
POINT2D translated_sphere[MAX_POINTS];
FACE sphere_faces[MAX_FACES];
VECTOR face_normals[MAX_FACES], light_source;

int visible_triangles;

static unsigned int sphere_rot_x = 0, sphere_rot_y = 0, sphere_rot_z = 0;


void make_unitvector(VECTOR* v,
    float x1, float y1, float z1, float x2, float y2, float z2)
{
    float lenght;
    float x, y, z;

    x = x2 - x1;
    y = y2 - y1;
    z = z2 - z1;

    lenght = sqrt(x * x + y * y + z * z);

    if (lenght) {
        v->x = x / lenght;
        v->y = y / lenght;
        v->z = z / lenght;
    } else {
        v->x = 0;
        v->y = 0;
        v->z = 0;
    }
}

void init_sphere(void)
{
    int i, j;
    POINT3D* point;
    float sphere_angle, circle_angle;
    FACE* face;

    point = &sphere[0];
    sphere_angle = 0;

    for (i = 0; i < POINTS_PER_CIRCLE; i++) {
        circle_angle = 0;

        for (j = 0; j < POINTS_PER_CIRCLE; j++) {
            point->x = sin(circle_angle) * SPHERE_RADIUS;
            point->y = cos(circle_angle) * SPHERE_RADIUS;
            point->z = sin(sphere_angle) * point->x;
            point->x *= cos(sphere_angle);

            point++;
            circle_angle += ANGLE_STEP;
        }

        sphere_angle += ANGLE_STEP;
    }
}

void init_sphere_faces(void)
{
    int i, j;
    FACE* face;

    face = &sphere_faces[0];

    for (j = (POINTS_PER_CIRCLE - 1); j > 0; j--) {
        for (i = 0; i < (POINTS_PER_CIRCLE - 1); i++) {
            face->v1 = (i * POINTS_PER_CIRCLE) + j;
            face->v2 = (i * POINTS_PER_CIRCLE) + (j - 1);
            face->v3 = ((i + 1) * POINTS_PER_CIRCLE) + (j - 1);
            face++;

            face->v1 = (i * POINTS_PER_CIRCLE) + j;
            face->v2 = ((i + 1) * POINTS_PER_CIRCLE) + (j - 1);
            face->v3 = ((i + 1) * POINTS_PER_CIRCLE) + j;
            face++;
        }
    }
}

void draw_sphere(unsigned char* frame_buffer)
{
    int i, x1, y1, x2, y2, x3, y3;
    FACE* face;
    POINT2D* point;
    VECTOR rotnrm, *rotated_normal, *light;
    unsigned char color;
    float dot_product;

    face = &sphere_faces[0];
    rotated_normal = &rotnrm;
    light = &light_source;
    visible_triangles = 0;

    for (i = 0; i < MAX_FACES; i++) {
        point = &translated_sphere[face->v1];
        x1 = point->x;
        y1 = point->y;
        point = &translated_sphere[face->v2];
        x2 = point->x;
        y2 = point->y;
        point = &translated_sphere[face->v3];
        x3 = point->x;
        y3 = point->y;

        if (((y1 - y3) * (x2 - x1) - (x1 - x3) * (y2 - y1)) > 0) {
            rotate_point(rotated_normal, &face_normals[i], sphere_rot_x, sphere_rot_y, sphere_rot_z);

            dot_product = (rotated_normal->x * light->x) + (rotated_normal->y * light->y) + (rotated_normal->z * light->z);

            if (dot_product > 1 || dot_product < 0)
                dot_product = 0;
            color = (unsigned char)(dot_product * 63);

            draw_triangle2(x1, y1, x2, y2, x3, y3, color, frame_buffer);
            visible_triangles++;
        }

        face++;
    }
}

void rotate_translate_sphere(void)
{
    int i;

    for (i = 0; i < MAX_POINTS; i++) {
        rotate_point(&rotated_sphere[i], &sphere[i], sphere_rot_x, sphere_rot_y, sphere_rot_z);
        translate_point(&translated_sphere[i], &rotated_sphere[i]);
    }
}

void init_sphere_palette(void)
{
    int i;
    unsigned char my_pal[768], *ptr;
    unsigned char t;

    dump_palette(my_pal);
    ptr = my_pal;

    for (i = 0; i < 64; i++) {
        *(ptr++) = i;
        *(ptr++) = i;
        *(ptr++) = 0;
    }
    for (i = 0; i < 8; i++) {
        t = (i * 3) + 30;
        *(ptr++) = t;
        *(ptr++) = t;
        *(ptr++) = t;
    }

    set_palette(my_pal);
}

void init_sphere_face_normals(void)
{
    int i;
    FACE* face;
    VECTOR* normal;
    float x_normal, y_normal, z_normal;
    POINT3D *p1, *p2, *p3;

    face = &sphere_faces[0];
    normal = &face_normals[0];

    for (i = 0; i < MAX_FACES; i++) {
        p1 = &sphere[face->v1];
        p2 = &sphere[face->v2];
        p3 = &sphere[face->v3];

        x_normal = (p2->y - p1->y) * (p1->z - p3->z) - (p2->z - p1->z) * (p1->y - p3->y);
        y_normal = (p2->z - p1->z) * (p1->x - p3->x) - (p2->x - p1->x) * (p1->z - p3->z);
        z_normal = (p2->x - p1->x) * (p1->y - p3->y) - (p2->y - p1->y) * (p1->x - p3->x);

        make_unitvector(normal, 0, 0, 0, x_normal, y_normal, z_normal);

        face++;
        normal++;
    }
}

void do_sphere(unsigned char* frame_buffer)
{
    char txt_buffer1[64], txt_buffer2[64];
    time_t before, after;
    unsigned long frames;
    float framerate;

    init_sphere();
    init_sphere_faces();
    init_sphere_face_normals();

    /* init light */

    make_unitvector(&light_source, 0, 0, -100, 0, 0, 0);

    init_sphere_palette();

    before = time(NULL);
    frames = 0;

    while (!is_key_pressed()) {
        timer_start(5);
        
        rotate_translate_sphere();
        draw_sphere(frame_buffer);

        /*
        itoa(visible_triangles, txt_buffer1, 10);
        strcpy(txt_buffer2, txt_buffer1);
        strcat(txt_buffer2, "/");

        frames++;
        after = time(NULL);

        itoa((after - before), txt_buffer1, 10);
        strcat(txt_buffer2, txt_buffer1);
        strcat(txt_buffer2, "/");

        ltoa(frames, txt_buffer1, 10);
        strcat(txt_buffer2, txt_buffer1);
        strcat(txt_buffer2, "/");

        if ((after - before)) {
            framerate = (float)(frames) / (float)(after - before);
            gcvt(framerate, 5, txt_buffer1);
            strcat(txt_buffer2, txt_buffer1);
        }

        put_string(txt_buffer2, 0, 1, TEXT_COLOR, frame_buffer);
        */
        do_scroll(199 - 8, frame_buffer);

        copy_buffer(frame_buffer);
        clear_buffer(frame_buffer);

        timer_wait();

        sphere_rot_x++;
        sphere_rot_x &= (MAX_DEGS - 1);

        sphere_rot_y += 3;
        sphere_rot_y &= (MAX_DEGS - 1);

        sphere_rot_z++;
        sphere_rot_z &= (MAX_DEGS - 1);
    }

    get_key_code();
}
