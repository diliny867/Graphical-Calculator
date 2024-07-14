#pragma once

#include <glm/glm.hpp>

#include "../myGL/VBO.h"

#include <iostream>
#include <vector>
#include <array>
#include <functional>
#include <sstream>
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

		inline static int screenWidth = 800;
		inline static int screenHeight = 600;
		inline static float xsize = 10.0f; //scale
		inline static float ysize = 10.0f;
		inline static float xcenter = 0.0f; //functions center position
		inline static float ycenter = 0.0f;

		constexpr inline static int calcPointsCount = 4000;

		inline static bool allDirty = false;
		bool dirty = false;

		ExprStrParser::Parser exprStrParser;

		Function();
		Function(int _screen_width, int _screen_height);
		std::array<glm::vec2, calcPointsCount+2>points;

		void setFunction(const std::string& str);
		glm::vec2 calcPointScrPos(glm::vec2 screenPos);
		float calcAtScrPos(glm::vec2 screenPos);
		static void setScreenSize(int width, int height);
		static void setSize(float x, float y);
		static void multSize(const float deltaX, const float deltaY);
		static glm::vec2 getSize();
		static void setCenter(float x, float y);
		static void setCenter(glm::vec2 center);
		static void incCenter(const float deltaX, const float deltaY);
		static void multCenter(const float deltaX, const float deltaY);
		static glm::vec2 getCenter();
		static glm::vec2 getCenterNDC();
		void recalculatePoints(); //function that recalculates points of function
		~Function() = default;
	};
}