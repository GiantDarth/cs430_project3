#ifndef CS430_PNM_H
#define CS430_PNM_H

#include <stddef.h>

#include "vector3d.h"

#define CS430_PNM_MIN 1
#define CS430_PNM_BITMAP_MAX 255
#define CS430_PNM_GREY_MAX 65535
#define CS430_PNM_FULL_MAX 65535
#define CS430_PNM_MAX_SUPPORTED 255
#define CS430_WIDTH_MIN 1
#define CS430_HEIGHT_MIN 1
#define CS430_MAX_LINE 70

typedef struct pnmHeader {
    int mode;
    size_t width;
    size_t height;
    size_t maxColorSize;
} pnmHeader;

typedef struct pixel {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} pixel;

static inline double clamp(double value, double min, double max) {
    if(value < min) {
        return min;
    }
    else if(value > max) {
        return max;
    }
}

static inline void pixel_clamp(pixel* pixel) {
    pixel->red = clamp(pixel->red, 0, 1);
    pixel->green = clamp(pixel->green, 0, 1);
    pixel->blue = clamp(pixel->blue, 0, 1);
}

static inline vector3d pixel2Vector3d(pixel pixel) {
    vector3d vector = {
        pixel.red,
        pixel.green,
        pixel.blue
    };

    return vector;
}

static inline pixel vector3d2pixel(vector3d vector) {
    pixel pixel = {
        vector.x,
        vector.y,
        vector.z
    };

    return pixel;
}

#endif // CS430_PNM_H
