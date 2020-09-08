/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "TextureBMP.h"
#include "Cylinder.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

using namespace std;

const float WIDTH = 20.0;
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 1000;
const int MAX_STEPS = 10;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const int checkSize = 200;
TextureBMP texture;
const bool antiAliasing = true;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ----------------------------------
//   Computes the colour value obtained by tracing a ray and finding its
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 backgroundCol(0);                     //Background colour = (0,0,0)
    glm::vec3 lightPos(-20, 40, 5);                 //Light's position
    glm::vec3 lightPos1(20, 40, 5);
    glm::vec3 color(0);
    SceneObject* obj;

    ray.closestPt(sceneObjects);                    //Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;       //no intersection
    obj = sceneObjects[ray.index];                  //object on which the closest point of intersection is found

    //Light vectors
    glm::vec3 lightVec = lightPos - ray.hit;
    glm::vec3 lightVec1 = lightPos1 - ray.hit;
    glm::vec3 normalVec = obj->normal(ray.hit);
    glm::vec3 lightUnitVec = glm::normalize(lightVec);
    glm::vec3 lightUnitVec1 = glm::normalize(lightVec1);

    //Chequered floor
    if (ray.index == 4) {
        int modx = (int)((ray.hit.x + checkSize) / 5) % 2;
        int modz = (int)((ray.hit.z + checkSize) / 5) % 2;

        if((modx && modz) || (!modx && !modz)) {
           color = glm::vec3(0.2,0.2,0.2);

        } else {
           color = glm::vec3(1,1,1);
        }
        obj->setColor(color);
    }

    //Texture map basketball onto sphere
    if(ray.index == 1) {
        glm::vec3 c(10.0, 10.0, -60.0); //Centre of the sphere
        glm::vec3 d = glm::normalize(ray.hit - c); //Unit vector from the intersection to the center of the sphere

        float texcoords = 0.5 + (atan2(d.x, d.z) / (2 * M_PI));
        float texcoordt = 0.5 - (asin(d.y) / M_PI);
        if(texcoords > 0 && texcoords < 1 &&
           texcoordt > 0 && texcoordt < 1)
        {
            color = texture.getColorAt(texcoords, texcoordt);
            obj->setColor(color);
        }
    }

    color = obj->lighting(lightPos, -ray.dir, ray.hit) + obj->lighting(lightPos1, -ray.dir, ray.hit);                     //Object's colour

    //Shadow 1 calculations
    Ray shadowRay(ray.hit, lightUnitVec);
    shadowRay.closestPt(sceneObjects);
    SceneObject *shadowObj;
    shadowObj = sceneObjects[shadowRay.index];

    if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {
        if (shadowObj->isRefractive() || shadowObj->isTransparent()) {
            color = 0.7f * (obj->getColor());
        } else {
            color = 0.2f * obj->getColor();
        }
    }

    //Shadow 2 calculations
    Ray shadow2Ray(ray.hit, lightUnitVec1);
    shadow2Ray.closestPt(sceneObjects);
    SceneObject *shadow2Obj;
    shadow2Obj = sceneObjects[shadow2Ray.index];

    if (shadow2Ray.index > -1 && shadow2Ray.dist < glm::length(lightVec1)) {
        if (shadow2Obj->isRefractive() || shadow2Obj->isTransparent()) {
            color = 0.7f * (obj->getColor());
        } else {
            color = 0.2f * obj->getColor();
        }
    }

    //Both shadows
    if ((shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) &&
    (shadow2Ray.index > -1 && shadow2Ray.dist < glm::length(lightVec1))) {
        if (shadow2Obj->isRefractive() || shadow2Obj->isTransparent()) {
            color = 0.35f * (obj->getColor());
        } else {
            color = 0.1f * obj->getColor();
        }
    }


    //Refractive glass sphere
    if (obj->isRefractive() && step < MAX_STEPS) {
        float eta = 1 / obj->getRefractiveIndex();
        glm::vec3 g = glm::refract(ray.dir, normalVec, eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta);
        Ray refrRay2(refrRay.hit, h);
        glm::vec3 r = trace(refrRay2, step + 1);
        color = 1.0f * r;
    }


    //Transparent objects
    if (obj->isTransparent() && step < MAX_STEPS) {
        Ray newRay(ray.hit, ray.dir);
        color = obj->getColor() + trace(newRay, step + 1);
    }


    if (obj->isReflective() && step < MAX_STEPS) {
        float rho = obj->getReflectionCoeff();
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }

    return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
    float xp, yp;  //grid point
    float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
    float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
    glm::vec3 eye(0., 0., 0.);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a tiny quad.

    for(int i = 0; i < NUMDIV; i++) //Scan every cell of the image plane
    {
        xp = XMIN + i*cellX;
        for(int j = 0; j < NUMDIV; j++)
        {
            yp = YMIN + j*cellY;

            glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);  //direction of the primary ray
            glm::vec3 col;

            if (antiAliasing) {

                //Split the pixel into four subpixels
                glm::vec3 directions[4] {
                    glm::vec3(xp+0.25*cellX, yp+0.25*cellY, -EDIST),
                    glm::vec3(xp+0.75*cellX, yp+0.25*cellY, -EDIST),
                    glm::vec3(xp+0.25*cellX, yp+0.75*cellY, -EDIST),
                    glm::vec3(xp+0.75*cellX, yp+0.75*cellY, -EDIST)
                };

                glm::vec3 colorSum = glm::vec3(0, 0, 0);

                for (int i = 0; i < 4; i++) {
                    Ray ray = Ray(eye, directions[i]);
                    //ray.normalize();
                    colorSum += trace(ray, 1);
                }
                //Now take the average of the colorSum to decide the color of the pixel
                col = colorSum * glm::vec3(0.25);

            } else {
                Ray ray = Ray(eye, dir);
                col = trace (ray, 1); //Trace the primary ray and get the colour value
            }


            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);             //Draw each cell with its color value
            glVertex2f(xp+cellX, yp);
            glVertex2f(xp+cellX, yp+cellY);
            glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}


void box(float length, float depth, float height, float x, float y, float z, glm::vec3 boxColor) {
    glm::vec3 a = glm::vec3(x, y, z);
    glm::vec3 b = glm::vec3(x, y+height, z);
    glm::vec3 c = glm::vec3(x+length, y+height, z);
    glm::vec3 d = glm::vec3(x+length, y, z);
    glm::vec3 e = glm::vec3(x, y, z-depth);
    glm::vec3 f = glm::vec3(x, y+height, z-depth);
    glm::vec3 g = glm::vec3(x+length, y+height, z-depth);
    glm::vec3 h = glm::vec3(x+length, y, z-depth);

    Plane *left = new Plane(e, a, b, f);
    Plane *right = new Plane(d, h, g, c);
    Plane *front = new Plane(a, d, c, b);
    Plane *back = new Plane(e, h, g, f);
    Plane *top = new Plane(b, c, g, f);
    Plane *bottom = new Plane(a, d, h, e);

    left->setColor(boxColor);
    right->setColor(boxColor);
    front->setColor(boxColor);
    back->setColor(boxColor);
    top->setColor(boxColor);
    bottom->setColor(boxColor);

    sceneObjects.push_back(left);
    sceneObjects.push_back(right);
    sceneObjects.push_back(front);
    sceneObjects.push_back(back);
    sceneObjects.push_back(top);
    sceneObjects.push_back(bottom);
}


//---This function initializes the scene -------------------------------------------
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographic projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

    texture = TextureBMP("ball.bmp");

    Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -110.0), 7.0);
    sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
    sphere1->setReflectivity(true, 0.8);
    sceneObjects.push_back(sphere1);         //Add sphere to scene objects

    Sphere *sphere2 = new Sphere(glm::vec3(10.0, 10.0, -60.0), 3.0);
    sphere2->setColor(glm::vec3(0, 1, 1));   //Set colour to cyan
    sceneObjects.push_back(sphere2);         //Add sphere to scene objects

    Sphere *sphere3 = new Sphere(glm::vec3(10.0, -10.0, -70.0), 5.0);
    sphere3->setRefractivity(true, 0.5, 1.5);
    sceneObjects.push_back(sphere3);           //Add sphere to scene objects

    Sphere *sphere4 = new Sphere(glm::vec3(5.0, 5.0, -80.0), 4.0);
    sphere4->setColor(glm::vec3(1, 0, 0));   //Set colour to red
    sceneObjects.push_back(sphere4);         //Add sphere to scene objects

    Plane *plane = new Plane (glm::vec3(-50., -15, -20), glm::vec3(50., -15, -20),
                              glm::vec3(50., -15, -200), glm::vec3(-50., -15, -200));
    plane->setSpecularity(false);
    sceneObjects.push_back(plane);

    Sphere *sphere5 = new Sphere(glm::vec3(0.0, -10.0, -60.0), 3.0);
    sphere5->setColor(glm::vec3(1, 0.0, 0.0));
    sphere5->setReflectivity(true, 0.1);
    sphere5->setTransparency(true);
    sceneObjects.push_back(sphere5);         //Add sphere to scene objects

    Cylinder *cylinder = new Cylinder (glm::vec3(-15.0, -15.0, -90.0), 3.0, 20.0,  glm::vec3(0.2, 0.2, 1.0));
    cylinder->setColor(glm::vec3(1, 0, 0));
    sceneObjects.push_back(cylinder);

    box(4, 5, 3, -11, -15, -63, glm::vec3(0, 1, 0));
}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(700, 700);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
