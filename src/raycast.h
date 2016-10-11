#ifndef CS430_RAYCAST_H
#define CS430_RAYCAST_H

#include <stddef.h>

#include "pnm.h"
#include "vector3d.h"

#define TYPE_SPHERE 0
#define TYPE_PLANE 1

typedef struct sceneObj {
    int type;
    pixel color;
    union {
        struct {
            vector3d pos;
            double radius;
        } sphere;
        struct {
            vector3d pos;
            vector3d normal;
        } plane;
        struct {
            vector3d pos;
            double radius;
            double height;
        } cylinder;
    };
} sceneObj;

typedef struct camera {
    float width;
    float height;
} camera;

typedef struct ray {
    vector3d origin;
    vector3d dir;
} ray;

void raycast(pixel* pixels, size_t width, size_t height, camera camera,
        const sceneObj* objs, size_t objsSize);

#endif // CS430_RAYCAST_H
