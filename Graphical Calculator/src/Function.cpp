#include "../include/Function.h"
#include "../include/FunctionSystem.h"

#include <utility>
#include <iostream>
#include <functional>

Function::Function() {
	id = idIndex++;
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
	//const std::size_t pointsCount = mainFunctionSystem->pointsCount;
	const std::size_t pointsCount = mainFunctionSystem->screen.x/mainFunctionSystem->funcPrecision;
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
	//const std::size_t pointsCount = mainFunctionSystem->pointsCount;
	if(mainFunctionSystem->funcPrecision < FunctionSystem::funcPrecisionMin) {
		return {0,nullptr};
	}
	const std::size_t pointsCount = mainFunctionSystem->screen.x/mainFunctionSystem->funcPrecision+4; // +/- 4 to account for outside of screen points
	const glm::vec2 size = mainFunctionSystem->size;
	//const float centerx = mainFunctionSystem->center.x;

	//const float indent = 1.0f/(static_cast<float>(pointsCount-4)*0.5);
	//float left = -centerx-1.0f-indent;

	const float indent = mainFunctionSystem->screen.x/(static_cast<float>(pointsCount-4)*0.5);
	float x = -mainFunctionSystem->screen.x*0.5f - indent;

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
		//pointsBuf.data[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(expr.Calculate(left * size.x)/size.y));

		pointsBuf.data[i] = glm::vec2(x, static_cast<float>(expr.Calculate(x)));
		//printf("x:%f y:%f\n", pointsBuf.data[i].x, pointsBuf.data[i].y);

		//left += indent;
		x += indent;
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
