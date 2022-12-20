#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 4) out;

uniform vec2 center;

void main() {
	gl_Position = vec4(-1.0, -center.y, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(1.0, -center.y, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();

	gl_Position = vec4(center.x, -1.0, 0.0, 1.0);
	EmitVertex();
	gl_Position = vec4(center.x, 1.0, 0.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
