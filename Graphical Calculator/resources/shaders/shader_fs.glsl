#version 330 core

uniform float time;
uniform vec2 resolution;

uniform float markerRadius;
uniform vec3 color;


void main(){
	//vec2 currCoord = gl_FragCoord.xy/resolution;
	//
	//for(int i=0;i<pointCount;i++){
	//	if(distance(gl_FragCoord.xy/resolution, vPoints[i]) <= markerRadius){
	//		gl_FragColor = vec4(vec3(1.0), 1.0);
	//		return;
	//	}
	//}
	//gl_FragColor = vec4(vec3(0.0), 1.0);

	gl_FragColor = vec4(color, 1.0);
}


