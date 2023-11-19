#include "../include/Function.h"
#include "../include/FunctionSystem.h"

#include <utility>
#include <iostream>
#include <functional>

//Function::Function(const int _screen_width, const int _screen_height):screen_width(_screen_width), screen_height(_screen_height), xsize(10.0f), ysize(10.0f), xcenter(0.0f), ycenter(0.0f), needs_update(false)
//	{ points.resize(calc_points_count+2); }
Function::Function(const int _screen_width, const int _screen_height) {
	//points.resize(mainFunctionSystem->calcPointsCount+2);
	id = idIndex++;
	points= {mainFunctionSystem->pointsCount, (glm::vec2*)calloc(mainFunctionSystem->pointsCount,sizeof(glm::vec2))};
}
Function::Function(): Function(800, 600) {
}

void Function::SetFunction(ExprStrParser::Expression expression) {
	functionExpression = std::move(expression);
}

glm::vec2 Function::CalcPointScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	return {screenPos.x, (functionExpression.Calculate((screenPos.x-mainFunctionSystem->center.x)*mainFunctionSystem->size.x)/mainFunctionSystem->size.y-mainFunctionSystem->center.y)};
}
float Function::CalcAtScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	return functionExpression.Calculate((screenPos.x-mainFunctionSystem->center.x)*mainFunctionSystem->size.x);
}

void Function::RecalculatePoints() {
	const std::size_t pointsCount = mainFunctionSystem->pointsCount;
	const glm::vec2 size = mainFunctionSystem->size;
	const float centerx = mainFunctionSystem->center.x;

	const float indent = 1.0f/(static_cast<float>(pointsCount)*0.5);
	float left = -centerx-1.0f-indent;

	std::lock_guard lg(m);

	for(std::size_t i=0; i<pointsCount; i++) {
		points.data[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(functionExpression.Calculate(left * size.x)/size.y));
		left += indent;
	}
}

PointsBuf Function::CalculatePoints() {
	//printf("start %d\n",id);
	const std::size_t pointsCount = mainFunctionSystem->pointsCount;
	const glm::vec2 size = mainFunctionSystem->size;
	const float centerx = mainFunctionSystem->center.x;

	const float indent = 1.0f/(static_cast<float>(pointsCount)*0.5);
	float left = -centerx-1.0f-indent;

	ExprStrParser::Expression expr;
	{
		std::lock_guard lg(m);
		expr = functionExpression.Copy();
	}

	while(recalculationBuffers>=recalculationBuffersMax) {
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	++recalculationBuffers;

	PointsBuf pointsBuf = {pointsCount,(glm::vec2*)malloc(sizeof(glm::vec2)*pointsCount)};

	for(std::size_t i=0; i<pointsCount; i++) {
		pointsBuf.data[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(expr.Calculate(left * size.x)/size.y));
		left += indent;
	}
	//printf("end %d\n",id);
	return pointsBuf;
}
void Function::SetPoints(PointsBuf buf) {
	std::lock_guard lg(m);
	points.size = buf.size;
	points.data = (glm::vec2*)realloc(points.data, sizeof(glm::vec2)*points.size);
	memcpy(points.data, buf.data, sizeof(glm::vec2)*points.size);
}
void Function::FreePointsBuf(PointsBuf buf) {
	free(buf.data);
	--recalculationBuffers;
}
