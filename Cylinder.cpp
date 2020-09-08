/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 pos, glm::vec3 dir)
{
    float x0minusxc = pos.x - center.x;
    float z0minuszc = pos.z - center.z;

    float a = pow(dir.x, 2) + pow(dir.z, 2);
    float b = 2 * ((dir.x * x0minusxc) + (dir.z * z0minuszc));
    float c = (x0minusxc * x0minusxc) + (z0minuszc * z0minuszc) - (radius * radius);
    float discrim = pow(b, 2) - 4 * (a * c);

    //Check if discriminant is valid
    if(fabs(discrim) < 0.001 || discrim < 0.0) {
        return -1.0;
    }

    //Get the roots of the equation
    float t1 = (-b - sqrt(discrim)) / (2 * a);
    float t2 = (-b + sqrt(discrim)) / (2 * a);

    float h1;
    float h2;
    if (t1 > 0.0f && t2 > 0.0f) {
        //Work out which root is closest and farthest.
        float close;
        float far;
        if (t1 < t2) {
            close = t1;
            far = t2;
        } else {
            close = t2;
            far = t1;
        }

        h1 = pos.y + dir.y * close; //The height of the point of intersection of close side
        h2 = pos.y + dir.y * far; //The height of the point of intersection of far side

        if (h1 < center.y + height && h1 > center.y) { //below the height of cylinder but above the bottom
            return close;
        }

        if (h2 < center.y + height && h2 > center.y) { //below the height of cylinder but above the bottom
            return far;
        }

    }
    return -1;
}



/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3(p.x - center.x, 0, p.z - center.z);
    n = glm::normalize(n);
    return n;
}
