#include <stdlib.h>
#include <time.h>

#include "low.h"
#include "clock.h"
#include "fontnew.h"
#include "math3d.h"

#include "scroll.h"
#include "flag3d.h"
#include "sphere.h"
#include "object3d.h"
#include "matrix.h"
#include "raytrace.h"
#include "bumpmain.h"

int main(void)
{
    unsigned char* buffer;
    
    if ((buffer = (unsigned char *)malloc(64000)) == NULL) {
        return 1;
    }

    srand(time(NULL));

    init_font();
    init_sincos();
    init_scroll();

    set_mode13h();

    clock_init();

    do_flag(buffer);
    do_sphere(buffer);
    do_object3d(buffer);
    do_matrix(buffer);
    do_raytrace(buffer);
    do_bump_mapping(buffer);

    unset_mode13h();
    clock_shutdown();

    free(buffer);

    return 0;
}
