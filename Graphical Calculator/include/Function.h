#pragma once

#include <glm/glm.hpp>

#include "../myGL/VBO.h"

#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <string>
#include <mutex>
#include "ExprStrParser.h"

class Function {
private:
	std::mutex m;
	//void recalculate_points_thread(const std::function<void(GLuint)>& callback, const GLuint vbo);

public:
	static inline class FunctionSystem* mainFunctionSystem = nullptr;

	bool needsPersonalUpdate = false;

	ExprStrParser::Expression functionExpression;

	Function();
	Function(int _screen_width, int _screen_height);
	std::vector<glm::vec2>points;

	void SetFunction(ExprStrParser::Expression expression);
	glm::vec2 CalcPointScrPos(glm::vec2 screenPos);
	float CalcAtScrPos(glm::vec2 screenPos);
	static glm::vec2 GetCenterNDC();
	void RecalculatePoints(); //function that recalculates points of function
	~Function() = default;
};

class MouseDot {
public:
	bool funcCaptured = false;
	Function* func = nullptr;
	bool byDistance = true;
	glm::vec2 screenPos = {0,0};
	glm::vec3 color = {1,1,1};
};
