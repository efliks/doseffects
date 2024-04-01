#include <math.h>
#include "math3d.h"

#define PERSPECTIVE 256.0
#define CORRECT_X 160.0
#define CORRECT_Y 100.0

float sine[MAX_DEGS], cosine[MAX_DEGS];


void init_sincos(void)
{
    int i;

    for (i = 0; i < MAX_DEGS; i++) {
        sine[i] = sin((float)i * M_PI / (MAX_DEGS / 2));
        cosine[i] = cos((float)i * M_PI / (MAX_DEGS / 2));
    }
}

void translate_point(POINT2D* dest, POINT3D* src)
{
    float temp = src->z + PERSPECTIVE;

    dest->x = ((src->x * PERSPECTIVE) / temp) + CORRECT_X;
    dest->y = ((src->y * PERSPECTIVE) / temp) + CORRECT_Y;
}

void rotate_point(POINT3D* dest, POINT3D* src,
    unsigned angle_x, unsigned angle_y, unsigned angle_z)
{
    float xr, yr, zr;

    dest->x = src->x;
    dest->y = src->y;
    dest->z = src->z;

    if (angle_x) {
        yr = (cosine[angle_x] * dest->y) - (sine[angle_x] * dest->z);
        zr = (sine[angle_x] * dest->y) + (cosine[angle_x] * dest->z);
        dest->y = yr;
        dest->z = zr;
    }

    if (angle_y) {
        xr = (cosine[angle_y] * dest->x) - (sine[angle_y] * dest->z);
        zr = (sine[angle_y] * dest->x) + (cosine[angle_y] * dest->z);
        dest->x = xr;
        dest->z = zr;
    }

    if (angle_z) {
        xr = (cosine[angle_z] * dest->x) - (sine[angle_z] * dest->y);
        yr = (sine[angle_z] * dest->x) + (cosine[angle_z] * dest->y);
        dest->x = xr;
        dest->y = yr;
    }
}

void mx_rotation_matrix(MATRIX result, short ax, short ay, short az)
{
    ax &= (MAX_DEGS - 1);
    ay &= (MAX_DEGS - 1);
    az &= (MAX_DEGS - 1);

    result[0][0] = cosine[ay] * cosine[az];
    result[1][0] = sine[ax] * sine[ay] * cosine[az] + cosine[ax] * (-sine[az]);
    result[2][0] = cosine[ax] * sine[ay] * cosine[az] + (-sine[ax]) * (-sine[az]);
    result[0][1] = cosine[ay] * sine[az];
    result[1][1] = sine[ax] * sine[ay] * sine[az] + cosine[ax] * cosine[az];
    result[2][1] = cosine[ax] * sine[ay] * sine[az] + (-sine[ax]) * cosine[az];
    result[0][2] = -sine[ay];
    result[1][2] = cosine[ay] * sine[ax];
    result[2][2] = cosine[ax] * cosine[ay];
}

void mx_rotate_points(POINT3D* dest_tab, POINT3D* src_tab,
    int n_points, MATRIX matrix)
{
    int i;
    POINT3D *src, *dest;

    src = src_tab;
    dest = dest_tab;

    for (i = 0; i < n_points; i++) {
        dest->x = src->x * matrix[0][0] + src->y * matrix[1][0] + src->z * matrix[2][0];
        dest->y = src->x * matrix[0][1] + src->y * matrix[1][1] + src->z * matrix[2][1];
        dest->z = src->x * matrix[0][2] + src->y * matrix[1][2] + src->z * matrix[2][2];

        src++;
        dest++;
    }
}

/* vector math */

void normalize_vector(VECTOR3D* v)
{
    float lenght;

    lenght = sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    if (lenght) {
        v->x /= lenght;
        v->y /= lenght;
        v->z /= lenght;
    } else {
        v->x = 0;
        v->y = 0;
        v->z = 0;
    }
}

void cross_product(VECTOR3D* result, VECTOR3D* v1, VECTOR3D* v2)
{
    result->x = v1->y * v2->z - v1->z * v2->y;
    result->y = v1->z * v2->x - v1->x * v2->z;
    result->z = v1->x * v2->y - v1->y * v2->x;
}

float dot_product(VECTOR3D* v1, VECTOR3D* v2)
{
    return v1->x * v2->x + v1->y * v2->y + v1->z * v2->z;
}

float vector_len(VECTOR3D* v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}
