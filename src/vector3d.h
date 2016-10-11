#ifndef CS430_VECTOR3D_H
#define CS430_VECTOR3D_H

#include <math.h>

typedef struct vector3d {
    double x;
    double y;
    double z;
} vector3d;

static inline vector3d vector3d_add(vector3d first, vector3d second) {
    vector3d result = { first.x + second.x, first.y + second.y, first.z + second.z };
    return result;
}

static inline vector3d vector3d_sub(vector3d first, vector3d second) {
    vector3d result = { first.x - second.x, first.y - second.y, first.z - second.z };
    return result;
}

static inline vector3d vector3d_scale(vector3d vector, double scaler) {
    vector3d result = { vector.x * scaler, vector.y * scaler, vector.z * scaler };
    return result;
}

static inline double vector3d_dot(vector3d first, vector3d second) {
    return first.x * second.x + first.y * second.y + first.z * second.z;
}

static inline vector3d vector3d_cross(vector3d first, vector3d second) {
    vector3d result = {
        first.y * second.z - first.z * second.y,
        first.z * second.x - first.x * second.z,
        first.x * second.y - first.y * second.x
    };
    return result;
}

static inline double vector3d_magnitude(vector3d vector) {
    return sqrt(pow(vector.x, 2) + pow(vector.y, 2) + pow(vector.z, 2));
}

static inline int vector3d_compare(vector3d first, vector3d second) {
    double firstMag = vector3d_magnitude(first);
    double secondMag = vector3d_magnitude(second);
    if(firstMag < secondMag) {
        return -1;
    }
    else if(firstMag > secondMag) {
        return 1;
    }
    else {
        return 0;
    }
}

static inline vector3d vector3d_normalize(vector3d vector) {
    double length = vector3d_magnitude(vector);
    vector3d normal = {
        vector.x / length,
        vector.y / length,
        vector.z / length
    };

    return normal;
}

#endif // CS430_VECTOR3D_H
