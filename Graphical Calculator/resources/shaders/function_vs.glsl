#version 330 core

layout (location = 0) in vec2 aPos;

uniform vec2 center;
uniform vec2 resolution;

uniform mat4 projection;

out TData{
    vec2 vLineCenter;
} outData;

void main(){
	vec4 pos = projection * vec4(aPos, 0.0, 1.0);
	pos.x+=center.x;
	pos.y-=center.y;
	outData.vLineCenter = (pos.xy + 1.0)/2.0*resolution;
	gl_Position = pos;
}