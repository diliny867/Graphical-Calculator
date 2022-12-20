#version 330 core

uniform float time;
uniform vec2 resolution;

uniform float markerRadius;
uniform vec3 color;


void main(){
	gl_FragColor = vec4(color, 1.0);
}


