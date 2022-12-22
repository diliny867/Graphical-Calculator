#include "../include/Function.h"

Function::Function(const int _screen_width, const int _screen_height):screen_width(_screen_width), screen_height(_screen_height), xsize(10.0f), ysize(10.0f), xcenter(0.0f), ycenter(0.0f), needs_update(false)
	{ points.resize(calc_points_count+2); }
Function::Function(): Function(800, 600) { }

void Function::setFunction(std::string& str){
	//std::cout<<str<<std::endl;
	expr_str_parser.parse(str);
}

void Function::updateScreenSize(const int _screen_width, const int _screen_height){
	screen_width = _screen_width;
	screen_height = _screen_height;
}

void Function::setSize(const float _xsize, const float _ysize){
	xsize = _xsize;
	ysize = _ysize;
	needs_update = true;
}
void Function::multSize(const float delta_xsize, const float delta_ysize){
	xsize *= delta_xsize;
	ysize *= delta_ysize;
	needs_update = true;
}
glm::vec2 Function::getSize() const{
	return {xsize, ysize};
}

void Function::setCenter(const float _xcenter, const float _ycenter){
	xcenter = _xcenter;
	ycenter = _ycenter;
	needs_update = true;
}
void Function::incCenter(const float delta_xcenter, const float delta_ycenter){
	xcenter += delta_xcenter;
	ycenter += delta_ycenter;
	needs_update = true;
}
void Function::multCenter(const float delta_xcenter, const float delta_ycenter){
	xcenter *= delta_xcenter;
	ycenter *= delta_ycenter;
	needs_update = true;
}
glm::vec2 Function::getCenter() const {
	return {xcenter, ycenter};
}
glm::vec2 Function::getCenterNDC() const {
	return (glm::vec2(xcenter, ycenter)+1.0f)/2.0f;
}

void Function::recalculatePoints() {
	const float indent = 1.0f/(static_cast<float>(calc_points_count)/2.0f);
	float left = -xcenter-1.0f-indent;
	std::lock_guard lg(m);
	for (int i = 0; i<calc_points_count+2; i++) {
		points[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), (expr_str_parser.calculate(left * xsize)/ysize));
		left += indent;
	}
}

//#include <thread>
//void Function::recalculate_points_thread(const std::function<void(GLuint)>& callback, const GLuint vbo) {
//	const float indent = 1.0f/(static_cast<float>(calc_points_count)/2.0f);
//	//std::cout<<indent<<std::endl;
//	float left = -xcenter-1.0f-indent;
//	//std::cout<<xsize<<" "<<ysize<<std::endl;
//	for (int i = 0; i<calc_points_count+2; i++) {
//		points[i] = glm::vec2((-1.0f-indent+static_cast<float>(i)*indent), (expr_str_parser.calculate(left * xsize)/ysize));
//		//std::cout<<-1.0f-indent+static_cast<float>(i)*indent<<std::endl;
//		//std::cout<<"x:" << points.back().x << " y:" << points.back().y <<std::endl;
//		left += indent;
//	}
//	callback(vbo);
//}


