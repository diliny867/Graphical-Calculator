#version 330 core

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 2) out;

void main (){
    if(distance(gl_in[0].gl_Position, gl_in[1].gl_Position) + distance(gl_in[2].gl_Position, gl_in[3].gl_Position) > distance(gl_in[1].gl_Position, gl_in[2].gl_Position)){
        gl_Position = gl_in[1].gl_Position;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        EmitVertex();
        EndPrimitive();
    }
}