#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 88) out;

uniform vec2 center;
uniform vec2 size;

uniform vec3 centerColor;
uniform vec3 gridColor;

out vec3 color;

float gridSize = 10.0; //making it as uniform breaks it somehow
float logDivision = 1.0/log(gridSize);

void main() {
	vec2 exp_ = ceil(logDivision*log(size));
	vec2 v = pow(vec2(gridSize),exp_);
	vec2 interval = (v/size)/10.0;
	vec2 offset = mod((abs(center)), interval); //without abs something goes wrong
	offset.x*=sign(center.x);
	offset.y*=sign(center.y);
	//vec2 sideOffset = 10.0/size-1.0;
	for(int i = -10; i <11; i++){ //need to draw 1 kinda redundant time, because grid sometimes was not completely drawn
		gl_Position = vec4(offset.x +(i*interval.x), -1.0, 0.0, 1.0);
		color = gridColor;
		EmitVertex();
		gl_Position = vec4(offset.x +(i*interval.x), 1.0, 0.0, 1.0);
		color = gridColor;
		EmitVertex();
		EndPrimitive();
		
		gl_Position = vec4(-1.0, -(offset.y +(i*interval.y)), 0.0, 1.0);
		color = gridColor;
		EmitVertex();
		gl_Position = vec4(1.0, -(offset.y +(i*interval.y)), 0.0, 1.0);
		color = gridColor;
		EmitVertex();
		EndPrimitive();
	}

	gl_Position = vec4(-1.0, -center.y, 0.0, 1.0);
	color = centerColor;
	EmitVertex();
	gl_Position = vec4(1.0, -center.y, 0.0, 1.0);
	color = centerColor;
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(center.x, -1.0, 0.0, 1.0);
	color = centerColor;
	EmitVertex();
	gl_Position = vec4(center.x, 1.0, 0.0, 1.0);
	color = centerColor;
	EmitVertex();
	EndPrimitive();
}
