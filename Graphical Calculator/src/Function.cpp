#include "../include/Function.h"

#include <iostream>
#include <functional>
#include <sstream>

#include "../include/FunctionSystem.h"

using namespace Application;

Function::Function() {}

void Function::setFunction(const std::string& str) {
	//std::cout<<str<<std::endl;
	exprStrParser.Parse(str);
}

glm::vec2 Function::calcPointScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	auto fs = FunctionSystem::GetInstance();
	const auto size = fs->GetSize();
	const auto center = fs->GetCenter();
	return {screenPos.x, (exprStrParser.Calculate((screenPos.x-center.x)*size.x)/size.y-center.y)};
}
float Function::calcAtScrPos(const glm::vec2 screenPos) {
	std::lock_guard lg(m);
	auto fs = FunctionSystem::GetInstance();
	const auto size = fs->GetSize();
	const auto center = fs->GetCenter();
	return exprStrParser.Calculate((screenPos.x-center.x)*size.x);
}

void Function::recalculatePoints() {
	unsigned int recalcId = currentRecalculationId.fetch_add(1);
	if(recalcId == UINT32_MAX) {
		recalcId = currentRecalculationId.fetch_add(1);
	}

	auto fs = FunctionSystem::GetInstance();
	const auto size = fs->GetSize();
	const auto center = fs->GetCenter();

	const float indent = 1.0f/(static_cast<float>(calcPointsCount)/2.0f);
	const float invSizeY = 1/size.y;
	float left = -center.x-1.0f-indent;
	std::lock_guard lg(m);
	for (int i = 0; i<calcPointsCount+2; i++) {
		if(completedRecalculationsMaxId.load()>=recalcId) {return;}
		points[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), static_cast<float>(exprStrParser.Calculate(left * size.x)*invSizeY));
		left += indent;
	}
	completedRecalculationsMaxId.store(recalcId);
}