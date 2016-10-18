#include <stdio.h>
#include <stdlib.h>

#include "json.h"
#include "raycast.h"
#include "pnm.h"
#include "write.h"

int main(int argc, char const *argv[]) {
    if(argc < 5) {
        fprintf(stderr, "usage: raycast width height /path/to/input.json "
            "/path/to/output.ppm\n");
        return 1;
    }
    jsonObj jsonObj = readScene(argv[3]);
    if(*(jsonObj.objs) == NULL) {
        return 0;
    }

    vector3d zeroVector = { 0 };

    for(size_t i = 0; jsonObj.objs[i] != NULL; i++) {
        if(jsonObj.objs[i]->type == TYPE_PLANE) {
            if(vector3d_compare(jsonObj.objs[i]->plane.normal, zeroVector) != 0) {
                jsonObj.objs[i]->plane.normal = vector3d_normalize(jsonObj.objs[i]->plane.normal);
            }
        }
    }

    char* endptr;
    size_t width = strtoul(argv[1], &endptr, 10);
    // If the first character is not empty and the set first invalid
    // character is empty, then the whole string is valid. (see 'man strtol')
    // Otherwise, part of the string is not a number.
    if(!(*(argv[1]) != '\0' && *endptr == '\0')) {
        fprintf(stderr, "Error: Invalid decimal value on channel\n");
        return 1;
    }
    size_t height = strtoul(argv[2], &endptr, 10);
    // If the first character is not empty and the set first invalid
    // character is empty, then the whole string is valid. (see 'man strtol')
    // Otherwise, part of the string is not a number.
    if(!(*(argv[2]) != '\0' && *endptr == '\0')) {
        fprintf(stderr, "Error: Invalid decimal value on channel\n");
        return 1;
    }

    pixel* pixels = malloc(sizeof(*pixels) * width * height);
    if(pixels == NULL) {
        fprintf(stderr, "Error: Memory allocation error\n");
        return 1;
    }

    raycast(pixels, width, height, jsonObj.camera, jsonObj.objs, jsonObj.lights);

    FILE* outputFd;
    if((outputFd = fopen(argv[4], "w")) == NULL) {
        perror("Error: Cannot open output file\n");
        return 1;
    }

    pnmHeader header = { 6, width, height, 255 };

    if(writeHeader(header, outputFd) < 0) {
        return 1;
    }
    if(writeBody(header, pixels, outputFd) < 0) {
        return 1;
    }

    return 0;
}
