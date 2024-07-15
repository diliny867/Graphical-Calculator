#pragma once

#include <glm/glm.hpp>

#include <cstdlib>

typedef unsigned long long ull;

template<typename T> struct CArr {
	std::size_t size;
	T* data;
};

typedef CArr<glm::vec2> PointsBuf;