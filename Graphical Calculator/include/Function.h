#pragma once

#include <glm/glm.hpp>

#include <array>
#include <string>
#include <mutex>

#include "ExprStrParser.h"

namespace Application{
	class Function {
	private:
		std::mutex m;
		std::atomic_uint32_t currentRecalculationId = 0;
		std::atomic_uint32_t completedRecalculationsMaxId = 0;

		//void recalculate_points_thread(const std::function<void(GLuint)>& callback, const GLuint vbo);

	public:
		inline static int numbersFloatPrecision = 1;

		constexpr inline static int calcPointsCount = 4000;

		bool dirty = false;

		ExprStrParser::Parser exprStrParser;

		Function();
		std::array<glm::vec2, calcPointsCount+2>points;

		void setFunction(const std::string& str);
		glm::vec2 calcPointScrPos(glm::vec2 screenPos);
		float calcAtScrPos(glm::vec2 screenPos);
		void recalculatePoints(); //function that recalculates points of function
		~Function() = default;
	};
}