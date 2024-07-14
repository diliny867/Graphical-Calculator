#include "../include/Function.h"

using namespace Application;

//Function::Function(const int _screen_width, const int _screen_height):screen_width(_screen_width), screen_height(_screen_height), xsize(10.0f), ysize(10.0f), xcenter(0.0f), ycenter(0.0f), needs_update(false)
//	{ points.resize(calc_points_count+2); }
Function::Function(const int _screen_width, const int _screen_height) {
	screenWidth = _screen_width;
	screenHeight = _screen_height;
}
Function::Function(): Function(800, 600) {}

void Function::setFunction(const std::string& str) {
	//std::cout<<str<<std::endl;
	exprStrParser.Parse(str);
}

glm::vec2 Function::calcPointScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	return {screenPos.x, (exprStrParser.Calculate((screenPos.x-xcenter)*xsize)/ysize-ycenter)};
}
float Function::calcAtScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	return exprStrParser.Calculate((screenPos.x-xcenter)*xsize);
}


void Function::setScreenSize(const int width, const int height) {
	screenWidth = width;
	screenHeight = height;
}

void Function::setSize(const float x, const float y) {
	xsize = x;
	ysize = y;
	//needs_update = true;
}
void Function::multSize(const float deltaX, const float deltaY) {
	xsize *= deltaX;
	ysize *= deltaY;
	//needs_update = true;
}
glm::vec2 Function::getSize() {
	return { xsize, ysize };
}

void Function::setCenter(const float x, const float y) {
	xcenter = x;
	ycenter = y;
	//needs_update = true;
}
void Function::setCenter(const glm::vec2 center) {
	xcenter = center.x;
	ycenter = center.y;
	//needs_update = true;
}
void Function::incCenter(const float deltaX, const float deltaY) {
	xcenter += deltaX;
	ycenter += deltaY;
	//needs_update = true;
}
void Function::multCenter(const float deltaX, const float deltaY) {
	xcenter *= deltaX;
	ycenter *= deltaY;
	//needs_update = true;
}
glm::vec2 Function::getCenter() {
	return {xcenter, ycenter};
}
glm::vec2 Function::getCenterNDC() {
	return (glm::vec2(xcenter, ycenter)+1.0f)/2.0f;
}

void Function::recalculatePoints() {
	unsigned int recalcId = currentRecalculationId.fetch_add(1);
	if(recalcId == UINT32_MAX) {
		recalcId = currentRecalculationId.fetch_add(1);
	}
	const float indent = 1.0f/(static_cast<float>(calcPointsCount)/2.0f);
	float left = -xcenter-1.0f-indent;
	std::lock_guard lg(m);
	for (int i = 0; i<calcPointsCount+2; i++) {
		if(completedRecalculationsMaxId.load()>=recalcId) {return;}
		points[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(exprStrParser.Calculate(left * xsize)/ysize));
		left += indent;
	}
	completedRecalculationsMaxId.store(recalcId);
}