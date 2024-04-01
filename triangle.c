#define SHIFT_CONST 6

//TODO Move to some common module
void draw_horizontal_line2(int x1, int y, int x2, unsigned char color,
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

void draw_triangle2(int x1, int y1, int x2, int y2, int x3, int y3,
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

            draw_horizontal_line2(line_x1, i, line_x2, color, buffer);
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

            draw_horizontal_line2(line_x1, i, line_x2, color, buffer);
        }

        fixedx1 += dx13;
        fixedx2 += dx23;
    }
}
