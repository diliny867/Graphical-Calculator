#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

//uniform vec2 center;
//uniform vec2 size;
//uniform vec2 updateOffset;
//uniform int index;
//
//float gridSize = 10.0;
//float logDivision = 1.0/log(gridSize);

void main(){
	//vec2 exp_ = ceil(logDivision*log(size));
	//vec2 v = pow(vec2(gridSize),exp_);
	//vec2 interval = (v/size)/10.0;
	//vec2 offset = mod((abs(center)), interval);
	//offset.x*=sign(center.x);
	//offset.y*=sign(center.y);

    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}