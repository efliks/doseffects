#ifndef _MATH3D_H
#define _MATH3D_H

#define MAX_DEGS 512
#define M_PI 3.14159265358979323846

typedef float MATRIX[3][3];

typedef struct
{
    float x, y, z;
} POINT3D, VECTOR3D, VECTOR;

typedef struct
{
    int x, y;
} POINT2D;

extern float sine[MAX_DEGS], cosine[MAX_DEGS];


void init_sincos();

void translate_point(POINT2D* dest, POINT3D* src);
void rotate_point(POINT3D* dest, POINT3D* src, unsigned angle_x, unsigned angle_y, unsigned angle_z);

void mx_rotation_matrix(MATRIX result, short ax, short ay, short az);
void mx_rotate_points(POINT3D* dest_tab, POINT3D* src_tab, int n_points, MATRIX matrix);

void normalize_vector(VECTOR3D* v);

void cross_product(VECTOR3D* result, VECTOR3D* v1, VECTOR3D* v2);
float dot_product(VECTOR3D* v1, VECTOR3D* v2);
float vector_len(VECTOR3D* v);

#endif // _MATH3D_H
