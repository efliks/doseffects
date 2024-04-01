/*
	Linear Texture Mapping Routines
	Mikolaj Felix 13/07/2001
	mfelix@polbox.com
*/

#define X1 0
#define Y1 1
#define X2 2
#define Y2 3

#define LEFT 0
#define RIGHT 1

int polygon_texcoords[200][4];
int polygon_edges[200][2];

unsigned char polygon_colors[200][2];


void textured_line(int x1, int x2, int y,
    int tx1, int ty1, int tx2, int ty2, unsigned char* texture, 
    unsigned char* buffer)
{
    int delta_x, delta_y, i, fixed_tx, fixed_ty, screen_off;
    int swap;

    if (x1 == x2)
        return;
    if (x1 > x2) {
        swap = x1;
        x1 = x2;
        x2 = swap;

        swap = tx1;
        tx1 = tx2;
        tx2 = swap;

        swap = ty1;
        ty1 = ty2;
        ty2 = swap;
    }

    delta_x = ((tx2 - tx1) << 7) / (x2 - x1);
    delta_y = ((ty2 - ty1) << 7) / (x2 - x1);
    fixed_tx = tx1 << 7;
    fixed_ty = ty1 << 7;

    screen_off = (y << 6) + (y << 8) + x1;

    for (i = x1; i < x2; i++) {
        buffer[screen_off] = texture[(fixed_tx >> 7) + (fixed_ty & 0xff80)];

        fixed_tx += delta_x;
        fixed_ty += delta_y;
        screen_off++;
    }
}

void textured_edge(int x1, int y1, int x2, int y2, int tx1, int ty1, int tx2, int ty2)
{
    int i, deltax, xfixed, x;
    int delta_tx, delta_ty, scan_x, scan_y;
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
        
        swap = ty1;
        ty1 = ty2;
        ty2 = swap;

        swap = tx1;
        tx1 = tx2;
        tx2 = swap;
    }

    deltax = ((x2 - x1) << 7) / (y2 - y1);
    xfixed = x1 << 7;

    delta_tx = ((tx2 - tx1) << 7) / (y2 - y1);
    delta_ty = ((ty2 - ty1) << 7) / (y2 - y1);
    scan_x = tx1 << 7;
    scan_y = ty1 << 7;

    for (i = y1; i < y2; i++) {
        x = xfixed >> 7;

        if (x < polygon_edges[i][LEFT]) {
            polygon_edges[i][LEFT] = x;
            polygon_texcoords[i][X1] = scan_x >> 7;
            polygon_texcoords[i][Y1] = scan_y >> 7;
        }

        if (x > polygon_edges[i][RIGHT]) {
            polygon_edges[i][RIGHT] = x;
            polygon_texcoords[i][X2] = scan_x >> 7;
            polygon_texcoords[i][Y2] = scan_y >> 7;
        }

        xfixed += deltax;
        scan_x += delta_tx;
        scan_y += delta_ty;
    }
}

void textured_polygon(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4,
    int tx1, int ty1, int tx2, int ty2, int tx3, int ty3, int tx4, int ty4,
    unsigned char* texture, unsigned char* buffer)
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

    textured_edge(x1, y1, x2, y2, tx1, ty1, tx2, ty2);
    textured_edge(x2, y2, x3, y3, tx2, ty2, tx3, ty3);
    textured_edge(x3, y3, x4, y4, tx3, ty3, tx4, ty4);
    textured_edge(x4, y4, x1, y1, tx4, ty4, tx1, ty1);

    for (i = min_y; i < max_y; i++) {
        textured_line(polygon_edges[i][LEFT], polygon_edges[i][RIGHT], i,
            polygon_texcoords[i][X1], polygon_texcoords[i][Y1],
            polygon_texcoords[i][X2], polygon_texcoords[i][Y2], texture, buffer);
    }
}
