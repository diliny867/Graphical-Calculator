#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

uniform vec2 center;

void main() {
	gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
	gl_Position = vec4(1.0, 1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(-1.0, -1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(1.0, -1.0, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
