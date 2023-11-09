#pragma once

#include "Function.h"

#include <glad/glad.h>//should always be first
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <queue>
#include <list>
#include <future>

#include "Common.h"

class FuncData {
public:
    Function function;
    std::string inputData;
    std::list<std::future<PointsBuf>> futures;
    glm::vec3 color = glm::vec3(1.0f);
    GLuint vbo = 0;
    GLuint vao = 0;
    bool show = true;
};
class FunctionSystem {
public:
    std::size_t pointsCount = 4000;
    double funcPrecision = 0.1;

	glm::vec2 screen;
	glm::vec2 center = {0,0};
	glm::vec2 size = {10,10};

    int numbersFloatPrecision = 1;

    ExprStrParser::Parser exprStrParser;

    bool functionsNeedUpdate = false;
    std::vector<FuncData*> functions;

	FunctionSystem(glm::vec2 screen_size);

};