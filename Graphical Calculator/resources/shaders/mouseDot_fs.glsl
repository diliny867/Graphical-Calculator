#version 330 core

uniform vec3 color;

void main(){
	vec2 circCoord = 2.0*gl_PointCoord-1.0;
	float alpha = (1 - pow(dot(circCoord, circCoord),2));
	gl_FragColor = vec4(color, alpha);
}