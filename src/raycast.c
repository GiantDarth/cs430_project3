#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846

#include "vector3d.h"
#include "raycast.h"

typedef struct shootObj {
    double t;
    sceneObj* obj;
} shootObj;

double sphere_intersection(ray ray, sceneObj* obj);
double plane_intersection(ray ray, sceneObj* obj);
double cylinder_intersection(ray ray, sceneObj* obj);

shootObj shoot(ray ray, sceneObj** objs);
pixel shade(ray ray, vector3d intersection, sceneObj* intersected, sceneObj** objs,
    sceneLight** lights);

vector3d getIntersection(vector3d origin, vector3d dir, double t);
vector3d getNormal(vector3d intersection, sceneObj* obj);
vector3d getColor(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light);
int inShadow(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light);
double getRadialAtten(vector3d intersection, sceneLight* light);
double getAngularAtten(vector3d intersection, sceneLight* light);
vector3d getDiffuse(vector3d intersection, sceneObj* closest, sceneLight* light);
vector3d getSpecular(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light);

void raycast(pixel* pixels, size_t width, size_t height, camera camera,
        sceneObj** objs, sceneLight** lights) {
    const vector3d center = { 0, 0, 1 };
    const double PIXEL_WIDTH = camera.width / width;
    const double PIXEL_HEIGHT = camera.height / height;

    vector3d point;
    shootObj closest;
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
            closest = shoot(ray, objs);
            if(closest.obj != NULL) {
                vector3d intersection = getIntersection(ray.origin, ray.dir, closest.t);
                pixels[y * width + x] = shade(ray, intersection, closest.obj,
                    objs, lights);
            }
        }
    }
}

shootObj shoot(ray ray, sceneObj** objs) {
    double closestValue = INFINITY;
    double t;

    shootObj closest = { 0 };

    for(size_t i = 0; objs[i] != NULL; i++) {
        switch(objs[i]->type) {
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
        if(t > 0 && t < closestValue) {
            closest.t = t;
            closest.obj = objs[i];
        }
    }

    return closest;
}

pixel shade(ray ray, vector3d intersection, sceneObj* closest, sceneObj** objs,
        sceneLight** lights) {
    vector3d sum = { 0 };
    for(size_t i = 0; lights[i] != NULL; i++) {
        if(!inShadow(ray, intersection, closest, objs)) {
            sum = vector3d_add(sum,
                getColor(ray, intersection, closest, lights[i]));
        }
    }

    pixel_clamp(&sum);
    pixel pixel = vector3d2pixel(sum);

    return pixel;
}

vector3d getIntersection(vector3d origin, vector3d dir, double t) {
    return vector3d_add(origin, vector3d_scale(dir, t));
}

vector3d getNormal(vector3d intersection, sceneObj* obj) {
    switch(obj->type) {
        case(TYPE_SPHERE):
            return vector3d_normalize(vector3d_sub(intersection, obj->sphere.pos));
        case(TYPE_PLANE):
            return obj->plane.normal;
        default:
            fprintf(stderr, "Error: Invalid obj type\n");
            exit(EXIT_FAILURE);
    }
}

vector3d getColor(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light) {
    double radialAtten = getRadialAtten(intersection, light);
    double angularAtten = getAngularAtten(intersection, light);

    vector3d sum = vector3d_add(
        getDiffuse(intersection, closest, light),
        getSpecular(ray, intersection, closest, light)
    );
    sum = vector3d_scale(sum, radialAtten * angularAtten);

    return sum;
}

int inShadow(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light) {
    // TODO
    return 0;
}

double getRadialAtten(vector3d intersection, sceneLight* light) {
    double distance = vector3d_distance(light->pos, intersection);

    if(distance == INFINITY) {
        return 1;
    }
    else {
        return (1 / (
            (light->radialAtten[2] * distance * distance) +
            (light->radialAtten[1] * distance) +
            light->radialAtten[0]
        ));
    }
}

double getAngularAtten(vector3d intersection, sceneLight* light) {
    // Not spot light
    if(light->theta == 0 || light->angularAtten == 0 || (
            light->pos.x == 0 && light->pos.y == 0 && light->pos.z == 0)) {
        return 1;
    }

    vector3d objVector = vector3d_normalize(vector3d_sub(intersection, light->pos));
    double cosPhi = vector3d_dot(objVector, light->dir);
    double cosTheta = cos(light->theta * PI / 180.0);
    if(cosPhi > cosTheta) {
        return 0;
    }

    return pow(vector3d_dot(objVector, light->pos), light->angularAtten);
}

vector3d getDiffuse(vector3d intersection, sceneObj* closest, sceneLight* light) {
    vector3d dir = vector3d_normalize(vector3d_sub(light->pos, intersection));
    vector3d normal = getNormal(intersection, closest);
    double cosAlpha = vector3d_dot(normal, dir);

    if(cosAlpha > 0) {
        return vector3d_scale(vector3d_product(closest->diffuse, light->color),
            cosAlpha);
    }
    else {
        return vector3d_zero();
    }
}

vector3d getSpecular(ray ray, vector3d intersection, sceneObj* closest, sceneLight* light) {
    vector3d normal = getNormal(intersection, closest);
    vector3d v = vector3d_scale(ray.dir, -1);
    double cosAlpha = vector3d_dot(normal, light->dir);
    vector3d r = vector3d_sub(vector3d_scale(normal, 2 * cosAlpha), light->dir);
    double cosBeta = vector3d_dot(v, r);

    if(cosBeta > 0 && cosAlpha > 0) {
        return vector3d_scale(
            vector3d_product(closest->specular, light->color),
            pow(cosBeta, closest->ns)
        );
    }
    else {
        return vector3d_zero();
    }
}

double plane_intersection(ray ray, sceneObj* obj) {
    double denominator = vector3d_dot(obj->plane.normal, ray.dir);
    // If the denominator is 0, then ray is parallel to plane
    if(denominator == 0) {
        return -1;
    }
    double t = - vector3d_dot(obj->plane.normal,
        vector3d_sub(ray.origin, obj->plane.pos)) / denominator;

    if(t > 0) {
        return t;
    }

    return -1;
}

double sphere_intersection(ray ray, sceneObj* obj) {
    // t_close = Rd * (C - Ro) closest apprach along ray
    // x_close = Ro + t_close*Rd closest point from circle center
    // d = ||x_close - C|| distance from circle center
    // a = sqrt(rad^2 - d^2)
    // t = t_close - a
    double t = vector3d_dot(ray.dir, vector3d_sub(obj->sphere.pos, ray.origin));
    vector3d point = vector3d_add(ray.origin, vector3d_scale(ray.dir, t));
    double magnitude = vector3d_magnitude(vector3d_sub(point, obj->sphere.pos));
    if(magnitude > obj->sphere.radius) {
        return -1;
    }
    else if(magnitude < obj->sphere.radius) {
        double a = sqrt(pow(obj->sphere.radius, 2) - pow(magnitude, 2));

        return t - a;
    }
    else {
        return t;
    }
}

double cylinder_intersection(ray ray, sceneObj* obj) {
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
        ray.dir.z * obj->cylinder.pos.x +
        ray.origin.z * ray.dir.z -
        ray.dir.z * obj->cylinder.pos.z
    );
    double c = pow(ray.origin.z, 2) -
        2 * ray.origin.x * obj->cylinder.pos.x +
        pow(obj->cylinder.pos.x, 2) + pow(ray.origin.z, 2) -
        2 * ray.origin.z * obj->cylinder.pos.z +
        pow(obj->cylinder.pos.z, 2);

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
