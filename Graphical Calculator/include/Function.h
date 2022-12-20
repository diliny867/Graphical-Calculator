#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <sstream>
#include <string>
#include "ExprStrParser.h"

//we assume half of the screen is xsize and ysize
class Function{
private:
	int screen_width;
	int screen_height;
	float xsize;
	float ysize;
	float xcenter;
	float ycenter;

	const int calc_points_count = 1600; //keep as multiple of 200

	ExprStrParser::Parser expr_str_parser;

	void recalculate_points_thread();
public:
	inline static int float_precision = 1;
	bool needs_update;
	Function();
	Function(int _screen_width, int _screen_height);
	std::vector<glm::vec2>points;
	
	void setFunction(std::string& str);
	void updateScreenSize(int _screen_width, int _screen_height);
	void updateSize(float _xsize, float _ysize);
	void multSize(const float delta_xsize, const float delta_ysize);
	glm::vec2 getSize() const;
	void setCenter(float _xcenter, float _ycenter);
	void incCenter(const float delta_xcenter, const float delta_ycenter);
	void multCenter(const float delta_xcenter, const float delta_ycenter);
	glm::vec2 getCenter() const;
	glm::vec2 getCenterNDC() const;
	void recalculatePoints();
};
