/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#ifndef H_CYLINDER
#define H_CYLINDER
#include <glm/glm.hpp>
#include "SceneObject.h"

/**
 * Defines a simple Cylinder located at 'center'
 * with the specified radius
 */
class Cylinder : public SceneObject
{

private:
    glm::vec3 center;
    float radius;
    float height;

public:

    Cylinder(glm::vec3 c, float r, float h, glm::vec3 col)
        : center(c), radius(r), height(h) {};

    float intersect(glm::vec3 posn, glm::vec3 dir);

    glm::vec3 normal(glm::vec3 p);

};

#endif //!H_CYLINDER
