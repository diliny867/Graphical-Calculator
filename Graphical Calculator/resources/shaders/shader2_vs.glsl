#version 330 core

layout (location = 0) in vec2 aPos;

uniform float ycenter;
//uniform float xmove;

void main(){
	gl_Position = vec4(aPos.x, aPos.y-ycenter, 1.0, 1.0);
	//gl_Position = vec4(aPos.x+xmove, aPos.y-ycenter, 1.0, 1.0);
}