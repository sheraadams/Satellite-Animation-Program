#pragma once
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <glad/glad.h>
#include<vector>
#include<string>
#include <iostream> 
#include <cstdlib>    
#include "stb_image.h"
#include"texture.h"
#include <glm/glm.hpp>

class Geometry
{
public:
    std::vector<GLfloat> GetBoxVertices(); 
    std::vector<GLfloat> GetSkyboxVertices();
    std::vector<GLfloat> GetCubeVertices();
    std::vector<GLfloat> GetPyramidVertices();
    std::vector<GLfloat> GetCompassVertices();
    std::vector<GLfloat> GetCompass2Vertices();
    glm::vec3* GetPointLightPositions();
    glm::vec3* GetLightPositions();
    glm::vec3 GetLightPos();
};


#endif