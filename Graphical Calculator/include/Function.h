#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <mutex>

#include "Common.h"
#include "ExprStrParser.h"

class Function {
private:
	std::mutex m;
	std::atomic_int recalculationBuffers = 0;
	constexpr static int recalculationBuffersMax = 10;
	inline static int idIndex = 0;
	int id = 0;
public:
	static inline class FunctionSystem* mainFunctionSystem = nullptr;

	bool needsPersonalUpdate = false;

	ExprStrParser::Expression functionExpression;

	Function();
	Function(int _screen_width, int _screen_height);
	//std::vector<glm::vec2>points;

	PointsBuf points;

	void SetFunction(ExprStrParser::Expression expression);
	glm::vec2 CalcPointScrPos(glm::vec2 screenPos);
	float CalcAtScrPos(glm::vec2 screenPos);
	void RecalculatePoints(); //function that recalculates points of function
	PointsBuf CalculatePoints(); //function that recalculates points of function
	void SetPoints(PointsBuf buf);
	void FreePointsBuf(PointsBuf buf);
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
