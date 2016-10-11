#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "raycast.h"

double sphere_intersection(ray ray, sceneObj obj);
double plane_intersection(ray ray, sceneObj obj);
double cylinder_intersection(ray ray, sceneObj obj);

const sceneObj* shoot(ray ray, const sceneObj* objs, size_t objsSize);
pixel shade(sceneObj intersected);

void raycast(pixel* pixels, size_t width, size_t height, camera camera,
        const sceneObj* objs, size_t objsSize) {
    const vector3d center = { 0, 0, 1 };
    const double PIXEL_WIDTH = camera.width / width;
    const double PIXEL_HEIGHT = camera.height / height;

    vector3d point;
    const sceneObj* intersected;
    // Initialize ray as origin and dir of { 0, 0, 0 }
    ray ray = { 0 };
    point.z = center.z;

    // Initialize all pixels to black
    memset(pixels, 0, sizeof(*pixels) * width * height);

    for(size_t y = 0; y < height; y++) {
        point.y = center.y - (camera.height / 2) + PIXEL_HEIGHT * (y + 0.5);
        // Adjust for image inversion
        point.y *= -1;
        for(size_t x = 0; x < width; x++) {
            point.x = center.x - (camera.width / 2) + PIXEL_WIDTH * (x + 0.5);
            ray.dir = vector3d_normalize(point);
            intersected = shoot(ray, objs, objsSize);
            if(intersected != NULL) {
                pixels[y * width + x] = shade(*intersected);
            }
        }
    }
}

const sceneObj* shoot(ray ray, const sceneObj* objs, size_t objsSize) {
    double closest = INFINITY;
    const sceneObj* closestObj = NULL;
    double t;

    for(size_t i = 0; i < objsSize; i++) {
        switch(objs[i].type) {
            case(TYPE_SPHERE):
                t = sphere_intersection(ray, objs[i]);
                break;
            case(TYPE_PLANE):
                t = plane_intersection(ray, objs[i]);
                break;
            default:
                fprintf(stderr, "Error: Invalid obj type\n");
                exit(EXIT_FAILURE);
        }
        if(t > 0 && t < closest) {
            closest = t;
            closestObj = &(objs[i]);
        }
    }

    return closestObj;
}

pixel shade(sceneObj intersected) {
    return intersected.color;
}

double plane_intersection(ray ray, sceneObj obj) {
    double denominator = vector3d_dot(obj.plane.normal, ray.dir);
    // If the denominator is 0, then ray is parallel to plane
    if(denominator == 0) {
        return -1;
    }
    double t = - vector3d_dot(obj.plane.normal,
        vector3d_sub(ray.origin, obj.plane.pos)) / denominator;

    if(t > 0) {
        return t;
    }

    return -1;
}

double sphere_intersection(ray ray, sceneObj obj) {
    // t_close = Rd * (C - Ro) closest apprach along ray
    // x_close = Ro + t_close*Rd closest point from circle center
    // d = ||x_close - C|| distance from circle center
    // a = sqrt(rad^2 - d^2)
    // t = t_close - a
    double t = vector3d_dot(ray.dir, vector3d_sub(obj.sphere.pos, ray.origin));
    vector3d point = vector3d_add(ray.origin, vector3d_scale(ray.dir, t));
    double magnitude = vector3d_magnitude(vector3d_sub(point, obj.sphere.pos));
    if(magnitude > obj.sphere.radius) {
        return -1;
    }
    else if(magnitude < obj.sphere.radius) {
        double a = sqrt(pow(obj.sphere.radius, 2) - pow(magnitude, 2));

        return t - a;
    }
    else {
        return t;
    }
}

double cylinder_intersection(ray ray, sceneObj obj) {
    // Step 1. Find the equation for the object you are innterested in
    // x^2 + y^2 = r^2
    //
    // Step 2. Paramaterize the equation with a center point if needed
    // (x - Cx)^2 + (z - Cz)^2 = r^2
    //
    // Step 3. Substitute the eq for a ray into our object equation.
    // (Rox + t * Rdx - Cx)^2 + (Roz + t * Rdz - Cz)^2 - r^2 = 0
    //
    // Step 4. Solve for t.
    //
    // Step 4a. Rewrite the equation (flatten).
    // -r^2 +
    // t^2 * Rdx^2 +
    // t^2 * Rdz^2 +
    // 2*t * Rox * Rdx -
    // 2*t * Rdz * Cx +
    // 2*t * Roz * Rdz -
    // 2*t * Rdz * Cz +
    // Rox^2 -
    // 2*Rox*Cx +
    // Cx^2 +
    // Roz^2 -
    // 2*Roz*Cz +
    // Cz^2 = 0
    //
    // Step 4b. Rewrite the equation in terms of t.
    // t^2 * (Rdx^2 + Rdz^2) +
    // t * (2 * (Rox * Rdx - Rdz * Cx + Roz * Rdz - Rdz * Cz)) +
    // Rox^2 - 2*Rox*Cx + Cx^2 + Roz^2 - 2*Roz*Cz + Cz^2 = 0
    //
    // Use the quadratic equation to solve for t
    //

    double a = pow(ray.dir.x, 2) + pow(ray.dir.x, 2);
    double b = 2 * (
        ray.origin.x * ray.dir.x -
        ray.dir.z * obj.cylinder.pos.x +
        ray.origin.z * ray.dir.z -
        ray.dir.z * obj.cylinder.pos.z
    );
    double c = pow(ray.origin.z, 2) -
        2 * ray.origin.x * obj.cylinder.pos.x +
        pow(obj.cylinder.pos.x, 2) + pow(ray.origin.z, 2) -
        2 * ray.origin.z * obj.cylinder.pos.z +
        pow(obj.cylinder.pos.z, 2);

    double determinant = pow(b, 2) - 4 * a *c;
    if (determinant < 0) {
        return -1;
    }

    determinant = sqrt(determinant);

    double t0 = (-b - determinant) / (2 * a);
    if(t0 > 0) {
        return t0;
    }

    double t1 = (-b + determinant) / (2 * a);
    if (t1 > 0) {
        return t1;
    }

    return -1;
}
