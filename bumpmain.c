/*
 * FlatGrd2 06/02/02
 * Mikolaj Felix a.k.a. Majuma
 * mfelix@polbox.com
 */

#include <stdlib.h>
#include <math.h>

#include "low.h"
#include "gfx.h"
#include "clock.h"
#include "scroll.h"

#include "bump.h"
#include "bumpobj.h"
#include "envmaps.h"

void random_phong_palette()
{
    int i;

    unsigned char r, g, b;
    unsigned char palette[768];
    
    float angle;
    float illumination;

    float diff_r = (float)(rand() % 20);
    float spec_r = (float)(rand() % 50);

    float diff_g = (float)(rand() % 20);
    float spec_g = (float)(rand() % 50);

    float diff_b = (float)(rand() % 20);
    float spec_b = (float)(rand() % 50);

    angle = M_PI / 2;

    for (i = 0; i <= MAX_PHONG_COLORS * 3; i += 3) {
        illumination = diff_r * cos(angle) + spec_r * pow(cos(angle), PHONG_SPEC_EXPONENT);
        r = (unsigned char)illumination;

        illumination = diff_g * cos(angle) + spec_g * pow(cos(angle), PHONG_SPEC_EXPONENT);
        g = (unsigned char)illumination;

        illumination = diff_b * cos(angle) + spec_b * pow(cos(angle), PHONG_SPEC_EXPONENT);
        b = (unsigned char)illumination;

        if (r > 31)
            r = 31;
        if (g > 63)
            g = 63;
        if (b > 31)
            b = 31;

        palette[i + 0] = r << 1;
        palette[i + 1] = g;
        palette[i + 2] = b << 1;

        angle -= PHONG_DELTA_ANGLE;
    }
    
    // font palette
    for (i = (MAX_PHONG_COLORS + 1) * 3; i < (MAX_PHONG_COLORS + 9) * 3; i += 3) {
        palette[i + 0] = (b = (i - MAX_PHONG_COLORS) + 30);
        palette[i + 1] = b;
        palette[i + 2] = b;
    }

    set_palette(palette);
}

int get_next_mode(int drawing_mode)
{
    int i;

    const int drawing_mode_table[][2] = {
        {FLAT_SHADING, GOURAUD_SHADING},
        {GOURAUD_SHADING, ENVIRONMENT_MAPPING}
    };

    for (i = 0; i < sizeof(drawing_mode_table) / sizeof(int) / 2; i++) {
        if (drawing_mode_table[i][0] == drawing_mode) {
            return drawing_mode_table[i][1];
        }
    }

    // next mode not found -> should exit
    return -1;
}

void do_bump_mapping(unsigned char* frame_buffer)
{
    OBJECT3D* obj;
    VECTOR3D light_source;

    int blur_mode = 0;
    int drawing_mode = FLAT_SHADING;
    
    obj = load_object3d("torus.3d");

    if (obj != NULL) {
        light_source.x = 0;
        light_source.y = 0;
        light_source.z = -100;
        normalize_vector(&light_source);
        
        // TODO Load palette from file
        random_phong_palette();
        enable_envmap();
    
        while (drawing_mode != -1) {
            timer_start(5);
    
            rotate_translate_object3d(obj, 1, 3, -1);
            draw_object3d(obj, &light_source, drawing_mode, frame_buffer);
    
            if (blur_mode) {
                do_blur(frame_buffer, 320, 200);
            }
        
            do_scroll(199 - 8, frame_buffer);

            copy_buffer(frame_buffer);
            clear_buffer(frame_buffer);

            timer_wait();

            if (is_key_pressed()) {
                get_key_code();
                drawing_mode = get_next_mode(drawing_mode);
            }
        }

        disable_envmap();
        unload_object3d(obj);
    }
}
