#pragma once

#include <glm/glm.hpp>

#include <myGL/VBO.h>

#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <string>
#include <mutex>
#include "ExprStrParser.h"

class Function {
private:
	std::mutex m;

	//void recalculate_points_thread(const std::function<void(GLuint)>& callback, const GLuint vbo);
public:
	inline static int float_precision = 1;

	inline static int screen_width = 800;
	inline static int screen_height = 600;
	inline static float xsize = 10.0f;
	inline static float ysize = 10.0f;
	inline static float xcenter = 0.0f;
	inline static float ycenter = 0.0f;

	const inline static int calc_points_count = 2000;

	inline static bool needs_update = false;

	bool needs_personal_update = false;

	ExprStrParser::Parser expr_str_parser;

	Function();
	Function(int _screen_width, int _screen_height);
	std::vector<glm::vec2>points;

	void setFunction(std::string& str);
	static void setScreenSize(int _screen_width, int _screen_height);
	static void setSize(float _xsize, float _ysize);
	static void multSize(const float delta_xsize, const float delta_ysize);
	static glm::vec2 getSize();
	static void setCenter(float _xcenter, float _ycenter);
	static void setCenter(glm::vec2 _center);
	static void incCenter(const float delta_xcenter, const float delta_ycenter);
	static void multCenter(const float delta_xcenter, const float delta_ycenter);
	static glm::vec2 getCenter();
	static glm::vec2 getCenterNDC();
	void recalculatePoints();
	~Function() = default;
};