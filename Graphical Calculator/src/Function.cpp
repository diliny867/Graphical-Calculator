#include <utility>

#include "../include/Function.h"
#include "../include/FunctionSystem.h"

//Function::Function(const int _screen_width, const int _screen_height):screen_width(_screen_width), screen_height(_screen_height), xsize(10.0f), ysize(10.0f), xcenter(0.0f), ycenter(0.0f), needs_update(false)
//	{ points.resize(calc_points_count+2); }
Function::Function(const int _screen_width, const int _screen_height) {
	points.resize(mainFunctionSystem->calcPointsCount+2);
}
Function::Function(): Function(800, 600) {}

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

glm::vec2 Function::GetCenterNDC() {
	return (mainFunctionSystem->center+1.0f)/2.0f;
}

void Function::RecalculatePoints() { 
	const float indent = 1.0f/(static_cast<float>(mainFunctionSystem->calcPointsCount)/2.0f);
	float left = -mainFunctionSystem->center.x-1.0f-indent;
	std::lock_guard lg(m);
	for (int i = 0; i<mainFunctionSystem->calcPointsCount+2; i++) {
		points[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(functionExpression.Calculate(left * mainFunctionSystem->size.x)/mainFunctionSystem->size.y));
		left += indent;
	}

}