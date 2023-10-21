#version 330 core

layout (location = 0) in vec2 aPos;

uniform float ycenter;
uniform vec2 resolution;

out TData{
    vec2 vLineCenter;
} outData;

void main(){
	vec2 pos = vec2(aPos.x, aPos.y-ycenter);
	outData.vLineCenter = (pos.xy + 1.0)/2.0*resolution;
	gl_Position = vec4(pos, 1.0, 1.0);
	
}