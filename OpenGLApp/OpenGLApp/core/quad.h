#ifndef QUAD_H
#define QUAD_H

#include <glad/glad.h>
#include <glm/glm.hpp>

class Quad
{
    public:
        static void instantiatePrimitive();
        static void freePrimitive();
        static unsigned int getVAO();

    private:
        static unsigned int VAO, VBO, EBO;
};
#endif