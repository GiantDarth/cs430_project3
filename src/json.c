#define __USE_MINGW_ANSI_STDIO 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "vector3d.h"
#include "json.h"

#define CAMERA_WIDTH_FLAG 0x1
#define CAMERA_HEIGHT_FLAG 0x2

#define DIFFUSE_FLAG 0x1
#define SPECULAR_FLAG 0x2
#define NS_FLAG 0x4

#define SPHERE_POS_FLAG 0x8
#define SPHERE_RAD_FLAG 0x10

#define PLANE_POS_FLAG 0x8
#define PLANE_NORMAL_FLAG 0x10

#define LIGHT_POS_FLAG 0x1
#define LIGHT_DIR_FLAG 0x2
#define LIGHT_COLOR_FLAG 0x4
#define LIGHT_THETA_FLAG 0x8
#define LIGHT_RAD_A0_FLAG 0x10
#define LIGHT_RAD_A1_FLAG 0x20
#define LIGHT_RAD_A2_FLAG 0x40
#define LIGHT_ANG_A0_FLAG 0x80

void errorCheck(int c, FILE* json, size_t line);
void tokenCheck(int c, char token, size_t line);
int jsonGetC(FILE* json, size_t* line);
void skipWhitespace(FILE* json, size_t* line);
void trailSpaceCheck(FILE* json, size_t* line);
char* nextString(FILE* json, size_t* line);
double nextNumber(FILE* json, size_t* line);
vector3d nextVector3d(FILE* json, size_t* line);
vector3d nextColor(FILE* json, size_t* line);

jsonObj readScene(const char* path) {
    FILE* json = fopen(path, "r");
    if(json == NULL) {
        perror("Error: Opening input\n");
        exit(EXIT_FAILURE);
    }

    sceneObj* obj;
    size_t objsSize = 0;
    sceneLight* light;
    size_t lightsSize = 0;

    jsonObj jsonObj = { 0 };

    int c;
    size_t line = 1;
    char* key, *type;
    int keyFlag;

    // Ignore beginning whitespace
    skipWhitespace(json, &line);

    c = jsonGetC(json, &line);
    tokenCheck(c, '[', line);

    skipWhitespace(json, &line);
    c = jsonGetC(json, &line);
    if(c == ']') {
        fprintf(stderr, "Warning: Line %zu: Empty array\n", line);

        trailSpaceCheck(json, &line);

        return jsonObj;
    }

    if(ungetc(c, json) == EOF) {
        fprintf(stderr, "Error: Line %zu: Read error\n", line);
        perror("");
        exit(EXIT_FAILURE);
    }

    do {
        skipWhitespace(json, &line);
        c = jsonGetC(json, &line);
        tokenCheck(c, '{', line);

        skipWhitespace(json, &line);
        c = jsonGetC(json, &line);
        // Empty object, skip to next object without allocating for empty one.
        if(c == '}') {
            fprintf(stderr, "Warning: Line %zu: Empty object\n", line);
            skipWhitespace(json, &line);
            continue;
        }
        else if(ungetc(c, json) == EOF) {
            fprintf(stderr, "Error: Line %zu: Read error\n", line);
            perror("");
            exit(EXIT_FAILURE);
        }

        key = nextString(json, &line);

        if(strcmp(key, "type") != 0) {
            fprintf(stderr, "Error: First key must be 'type'\n");
            exit(EXIT_FAILURE);
        }

        skipWhitespace(json, &line);
        c = jsonGetC(json, &line);
        tokenCheck(c, ':', line);

        skipWhitespace(json, &line);
        type = nextString(json, &line);

        if(strcmp(type, "plane") == 0 || strcmp(type, "sphere") == 0) {
            if((obj = malloc(sizeof(*obj))) == NULL) {
                fprintf(stderr, "Error: Line %zu: Memory reallocation error\n",
                    line);
                perror("");
                exit(EXIT_FAILURE);
            }

            memset(obj, 0, sizeof(*obj));

            if(strcmp(type, "plane") == 0) {
                obj->type = TYPE_PLANE;
            }
            else if(strcmp(type, "sphere") == 0) {
                obj->type = TYPE_SPHERE;
            }

            obj->ns = DEFAULT_NS;
            obj->specular.x = 1;
            obj->specular.z = 1;
            obj->specular.y = 1;

            jsonObj.objs = realloc(jsonObj.objs, ++objsSize *
                sizeof(*(jsonObj.objs)));
            jsonObj.objs[objsSize - 1] = obj;
        }
        else if(strcmp(type, "light") == 0) {
            if((light = malloc(sizeof(*light))) == NULL) {
                fprintf(stderr, "Error: Line %zu: Memory reallocation error\n",
                    line);
                perror("");
                exit(EXIT_FAILURE);
            }

            memset(light, 0, sizeof(*light));

            light->radialAtten[2] = 1;

            jsonObj.lights = realloc(jsonObj.lights, ++lightsSize *
                sizeof(*(jsonObj.lights)));
            jsonObj.lights[lightsSize - 1] = light;
        }
        else if(strcmp(type, "camera") != 0) {
            fprintf(stderr, "Error: Line %zu: Unknown type %s", line,
                type);
            exit(EXIT_FAILURE);
        }

        keyFlag = 0;

        skipWhitespace(json, &line);
        while((c = jsonGetC(json, &line)) == ',') {
            skipWhitespace(json, &line);
            // Get key
            key = nextString(json, &line);

            // Get ':' token
            skipWhitespace(json, &line);
            c = jsonGetC(json, &line);
            tokenCheck(c, ':', line);

            skipWhitespace(json, &line);
            // TODO: Find way to remove redundant code
            if(strcmp(type, "camera") == 0) {
                if(strcmp(key, "width") == 0) {
                    if(keyFlag & CAMERA_WIDTH_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'width' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= CAMERA_WIDTH_FLAG;

                    jsonObj.camera.width = nextNumber(json, &line);
                    if(jsonObj.camera.width < 0) {
                        fprintf(stderr, "Error: Line %zu: Width cannot be negative\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                }
                else if(strcmp(key, "height") == 0) {
                    if(keyFlag & CAMERA_HEIGHT_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'height' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= CAMERA_HEIGHT_FLAG;

                    jsonObj.camera.height = nextNumber(json, &line);
                    if(jsonObj.camera.height < 0) {
                        fprintf(stderr, "Error: Line %zu: Height cannot be negative\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    fprintf(stderr, "Error: Line %zu: Key '%s' not supported "
                        "under 'camera'\n", line, key);
                    exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(type, "sphere") == 0) {
                if(strcmp(key, "position") == 0) {
                    if(keyFlag & SPHERE_POS_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'position' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= SPHERE_POS_FLAG;

                    obj->sphere.pos = nextVector3d(json, &line);
                }
                else if(strcmp(key, "radius") == 0) {
                    if(keyFlag & SPHERE_RAD_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'radius' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }

                    obj->sphere.radius = nextNumber(json, &line);
                    if(obj->sphere.radius < 0) {
                        fprintf(stderr, "Error: Line %zu: Radius cannot be "
                            "negative\n", line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= SPHERE_RAD_FLAG;
                }
                else if(strcmp(key, "diffuse_color") == 0) {
                    if(keyFlag & DIFFUSE_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'diffuse_color' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= DIFFUSE_FLAG;

                    obj->diffuse = nextColor(json, &line);
                }
                else if(strcmp(key, "specular_color") == 0) {
                    if(keyFlag & SPECULAR_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'specular_color' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= SPECULAR_FLAG;

                    obj->specular = nextColor(json, &line);
                }
                else {
                    fprintf(stderr, "Error: Line %zu: Key '%s' not supported "
                        "under 'sphere'\n", line, key);
                    exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(type, "plane") == 0) {
                if(strcmp(key, "position") == 0) {
                    if(keyFlag & PLANE_POS_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'position' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= PLANE_POS_FLAG;

                    obj->plane.pos = nextVector3d(json, &line);
                }
                else if(strcmp(key, "normal") == 0) {
                    if(keyFlag & PLANE_NORMAL_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'normal' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= PLANE_NORMAL_FLAG;

                    obj->plane.normal = nextVector3d(json, &line);
                }
                else if(strcmp(key, "diffuse_color") == 0) {
                    if(keyFlag & DIFFUSE_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'diffuse_color' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= DIFFUSE_FLAG;

                    obj->diffuse = nextColor(json, &line);
                }
                else if(strcmp(key, "specular_color") == 0) {
                    if(keyFlag & SPECULAR_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'specular_color' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= SPECULAR_FLAG;

                    obj->specular = nextColor(json, &line);
                }
                else {
                    fprintf(stderr, "Error: Line %zu: Key '%s' not supported "
                        "under 'plane'\n", line, key);
                    exit(EXIT_FAILURE);
                }
            }
            else if(strcmp(type, "light") == 0) {
                if(strcmp(key, "position") == 0) {
                    if(keyFlag & LIGHT_POS_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'position' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_POS_FLAG;

                    light->pos = nextVector3d(json, &line);
                }
                else if(strcmp(key, "direction") == 0) {
                    if(keyFlag & LIGHT_DIR_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'direction' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_DIR_FLAG;

                    light->dir = nextVector3d(json, &line);
                }
                else if(strcmp(key, "color") == 0) {
                    if(keyFlag & LIGHT_COLOR_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'color' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_COLOR_FLAG;

                    light->color = nextColor(json, &line);
                }
                else if(strcmp(key, "theta") == 0) {
                    if(keyFlag & LIGHT_THETA_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'theta' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_THETA_FLAG;

                    light->theta = nextNumber(json, &line);
                }
                else if(strcmp(key, "radial-a0") == 0) {
                    if(keyFlag & LIGHT_RAD_A0_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'radial-a0' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_RAD_A0_FLAG;

                    light->radialAtten[0] = nextNumber(json, &line);
                }
                else if(strcmp(key, "radial-a1") == 0) {
                    if(keyFlag & LIGHT_RAD_A1_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'radial-a1' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_RAD_A1_FLAG;

                    light->radialAtten[1] = nextNumber(json, &line);
                }
                else if(strcmp(key, "radial-a2") == 0) {
                    if(keyFlag & LIGHT_RAD_A2_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'radial-a2' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_RAD_A2_FLAG;

                    light->radialAtten[2] = nextNumber(json, &line);
                }
                else if(strcmp(key, "angular-a0") == 0) {
                    if(keyFlag & LIGHT_ANG_A0_FLAG) {
                        fprintf(stderr, "Error: Line %zu: 'angular-a0' already defined\n",
                            line);
                        exit(EXIT_FAILURE);
                    }
                    keyFlag |= LIGHT_ANG_A0_FLAG;

                    light->angularAtten = nextNumber(json, &line);
                }
            }

            skipWhitespace(json, &line);
        }

        if(strcmp(type, "camera") == 0) {
            if(!(keyFlag & CAMERA_WIDTH_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'camera' missing 'width' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & CAMERA_HEIGHT_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'camera' missing 'height' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(type, "sphere") == 0) {
            if(!(keyFlag & SPHERE_RAD_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'sphere' missing 'radius' "
                    "property missing\n", line);

                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & SPHERE_POS_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'sphere' missing 'position' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & DIFFUSE_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'sphere' missing 'diffuse_color' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(type, "plane") == 0) {
            if(!(keyFlag & PLANE_POS_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'plane' missing 'position' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & PLANE_NORMAL_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'plane' missing 'normal' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & DIFFUSE_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'plane' missing 'diffuse_color' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
        }
        else if(strcmp(type, "light") == 0) {
            if(!(keyFlag & LIGHT_POS_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'light' missing 'position' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
            if(!(keyFlag & LIGHT_COLOR_FLAG)) {
                fprintf(stderr, "Error: Line %zu: 'light' missing 'color' "
                    "property missing\n", line);
                exit(EXIT_FAILURE);
            }
        }

        tokenCheck(c, '}', line);

        skipWhitespace(json, &line);
    }
    while((c = jsonGetC(json, &line)) == ',');

    tokenCheck(c, ']', line);

    trailSpaceCheck(json, &line);

    jsonObj.objs = realloc(jsonObj.objs, (objsSize + 1) * sizeof(*(jsonObj.objs)));
    jsonObj.objs[objsSize] = NULL;
    jsonObj.lights = realloc(jsonObj.lights, (lightsSize + 1) * sizeof(*(jsonObj.lights)));
    jsonObj.lights[lightsSize] = NULL;

    return jsonObj;
}

void errorCheck(int c, FILE* fp, size_t line) {
    if(c == EOF) {
        if(feof(fp)) {
            fprintf(stderr, "Error: Line %zu: Premature end-of-file\n", line);
            exit(EXIT_FAILURE);
        }
        else if(ferror(fp)) {
            fprintf(stderr, "Error: Line %zu: Read error\n", line);
            perror("");
            exit(EXIT_FAILURE);
        }
    }
}

void tokenCheck(int c, char token, size_t line) {
    if(c != token) {
        fprintf(stderr, "Error: Line %zu: Expected '%c'\n", line, token);
        exit(EXIT_FAILURE);
    }
}

int jsonGetC(FILE* json, size_t* line) {
    int c = fgetc(json);
    errorCheck(c, json, *line);

    if(c == '\n') {
        *line += 1;
    }

    return c;
}

void skipWhitespace(FILE* json, size_t* line) {
    int c;

    c = jsonGetC(json, line);
    while(isspace(c)) {
        c = jsonGetC(json, line);
    }

    if(ungetc(c, json) == EOF) {
        fprintf(stderr, "Error: Line %zu: Read error\n", *line);
        perror("");
        exit(EXIT_FAILURE);
    }
}

void trailSpaceCheck(FILE* json, size_t* line) {
    int c;

    // Manually get trailing whitespace
    while((c = fgetc(json)) != EOF && isspace(c)) {
        if(c == '\n') {
            *line += 1;
        }
    }

    if(c != EOF) {
        fprintf(stderr, "Error: Line %zu: Unkown token at end-of-file\n", *line);
        exit(EXIT_FAILURE);
    }
    else if(!feof(json) && ferror(json)) {
        fprintf(stderr, "Error: Line %zu: Read error\n", *line);
        perror("");
        exit(EXIT_FAILURE);
    }
}

char* nextString(FILE* json, size_t* line) {
    size_t bufferSize = 64;
    size_t oldSize = bufferSize;
    char* buffer = malloc(bufferSize);
    if(buffer == NULL) {
        fprintf(stderr, "Error: Line %zu: Memory allocation error\n", *line);
        perror("");
        exit(EXIT_FAILURE);
    }
    int c;
    size_t i;

    c = jsonGetC(json, line);
    tokenCheck(c, '"', *line);

    i = 0;
    while(i < bufferSize - 1 && (c = jsonGetC(json, line)) != '"') {
        if(i == bufferSize - 2) {
            bufferSize *= 2;
            // Integer overflow
            if(oldSize != 0 && bufferSize / oldSize != 2) {
                fprintf(stderr, "Error: Line %zu: Integer overflow on size\n",
                    *line);
                exit(EXIT_FAILURE);
            }

            oldSize = bufferSize;
            buffer = realloc(buffer, bufferSize);
            if(buffer == NULL) {
                fprintf(stderr, "Error: Line %zu: Memory reallocation error\n",
                    *line);
                perror("");
                exit(EXIT_FAILURE);
            }
        }
        buffer[i++] = c;
    }
    buffer[i] = '\0';

    return buffer;
}

double nextNumber(FILE* json, size_t* line) {
    double value;
    int status = fscanf(json, "%lf", &value);
    errorCheck(status, json, *line);
    if(status < 1) {
        fprintf(stderr, "Error: Line %zu: Invalid number\n", *line);
        exit(EXIT_FAILURE);
    }

    if(errno == ERANGE) {
        if(value == 0) {
            fprintf(stderr, "Error: Line %zu: Number underflow\n", *line);
            exit(EXIT_FAILURE);
        }
        if(value == HUGE_VAL || value == -HUGE_VAL) {
            fprintf(stderr, "Error: Line %zu: Number overflow\n", *line);
            exit(EXIT_FAILURE);
        }
    }

    return value;
}

vector3d nextVector3d(FILE* json, size_t* line) {
    vector3d vector;

    int c = jsonGetC(json, line);
    tokenCheck(c, '[', *line);

    skipWhitespace(json, line);
    vector.x = nextNumber(json, line);

    skipWhitespace(json, line);
    c = jsonGetC(json, line);
    tokenCheck(c, ',', *line);

    skipWhitespace(json, line);
    vector.y = nextNumber(json, line);

    skipWhitespace(json, line);
    c = jsonGetC(json, line);
    tokenCheck(c, ',', *line);

    skipWhitespace(json, line);
    vector.z = nextNumber(json, line);

    skipWhitespace(json, line);
    c = jsonGetC(json, line);
    tokenCheck(c, ']', *line);

    return vector;
}

vector3d nextColor(FILE* json, size_t* line) {
    vector3d color = nextVector3d(json, line);

    if(color.x < 0 || color.y < 0 || color.z < 0) {
        fprintf(stderr, "Error: Line %zu: Color must be at least 0.0.\n",
            *line);
        exit(EXIT_FAILURE);
    }

    return color;
}
