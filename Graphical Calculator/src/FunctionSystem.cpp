#include "../include/FunctionSystem.h"

using namespace Application;

FunctionSystem::FunctionSystem(const glm::vec2& screenSize): screen(screenSize) {
	instance = this;
}
FunctionSystem::FunctionSystem(): FunctionSystem({800, 600}) {
}

void FunctionSystem::SetScreen(const glm::vec2& size) {
	std::lock_guard lg(m);
	screen = size;
}
glm::vec2 FunctionSystem::GetScreen() {
	std::lock_guard lg(m);
	return screen;
}

void FunctionSystem::SetSize(const glm::vec2& size) {
	std::lock_guard lg(m);
	this->size = size;
	//needs_update = true;
}
void FunctionSystem::IncSize(const glm::vec2& delta) {
	std::lock_guard lg(m);
	size += delta;
	//needs_update = true;
}void FunctionSystem::MultSize(const glm::vec2& delta) {
	std::lock_guard lg(m);
	size *= delta;
	//needs_update = true;
}
glm::vec2 FunctionSystem::GetSize() {
	std::lock_guard lg(m);
	return size;
}

void FunctionSystem::SetCenter(const glm::vec2& center) {
	std::lock_guard lg(m);
	this->center = center;
	//needs_update = true;
}
void FunctionSystem::IncCenter(const glm::vec2& delta) {
	std::lock_guard lg(m);
	center += delta;
	//needs_update = true;
}
void FunctionSystem::MultCenter(const glm::vec2& delta) {
	std::lock_guard lg(m);
	center *= delta;
	//needs_update = true;
}
glm::vec2 FunctionSystem::GetCenter() {
	std::lock_guard lg(m);
	return center;
}
glm::vec2 FunctionSystem::GetCenterNDC() {
	std::lock_guard lg(m);
	return (center+1.0f)*0.5f;
}