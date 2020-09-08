cmake_minimum_required(VERSION 2.8)

project(RayTracingAssignment)

add_executable(RayTracer.out RayTracer.cpp Cylinder.cpp Ray.cpp SceneObject.cpp Sphere.cpp Plane.cpp TextureBMP.cpp)

find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(glm REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} "/usr/local/Cellar/glm/0.9.9.8/include" )

target_link_libraries( RayTracer.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLM_LIBRARY} )

