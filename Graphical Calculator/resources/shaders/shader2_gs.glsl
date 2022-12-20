#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 202) out;

uniform vec2 vPoints[202];
uniform int pointCount;
uniform float ycenter;


void main() {
	for(int i=0;i<pointCount;i++){
		gl_Position = vec4(vPoints[i].x, vPoints[i].y-ycenter, 0.0, 1.0);
		EmitVertex();
	}
	EndPrimitive();

	//gl_Position = vec4(0.0,0.0,0.0,1.0);
	//EmitVertex();
	//gl_Position = vec4(0.0,1.0,0.0,1.0);
	//EmitVertex();
	//gl_Position = vec4(1.0,1.0,0.0,1.0);
	//EmitVertex();
	//gl_Position = vec4(1.0,0.0,0.0,1.0);
	//EmitVertex();
	//EndPrimitive();

	//gl_Position = gl_in[1].gl_Position;
	//EmitVertex();
	//gl_Position = gl_in[2].gl_Position;
	//EmitVertex();
	//EndPrimitive();

	//gl_Position = gl_in[0].gl_Position;
	//EmitVertex();
	//gl_Position = gl_in[1].gl_Position;
	//EmitVertex();
	//gl_Position = gl_in[2].gl_Position;
	//EmitVertex();
	//EndPrimitive();
}
