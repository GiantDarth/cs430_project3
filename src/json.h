#ifndef CS430_JSON_H
#define CS430_JSON_H

#include <stddef.h>

#include "pnm.h"
#include "raycast.h"

typedef struct jsonObj {
    camera camera;
    sceneObj** objs;
    sceneLight** lights;
} jsonObj;

jsonObj readScene(const char* path);

#endif // CS430_JSON_H
