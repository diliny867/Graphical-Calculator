#version 330 core

uniform vec3 color;
uniform vec2 resolution;
uniform float radius;


float circle(vec2 pos, float r){
	float value = distance(pos, vec(0.5));
	return step(radius, value);
}

void main(){
	vec2 coord = gl_FragCoord.xy/resolution;
	float circle = circleShape(coord, radius);
	if(circle <=0.01){
		discard;
	}else{
		gl_FragColor = vec4(color, 1.0);
	}
}


