#version 330 core

layout (lines_adjacency) in;
layout (line_strip, max_vertices = 2) out;

in TData{
    vec2 vLineCenter;
} inData[];

out TData{
    vec2 vLineCenter;
} outData;

void main (){
    if(distance(gl_in[0].gl_Position, gl_in[1].gl_Position) + distance(gl_in[2].gl_Position, gl_in[3].gl_Position) > distance(gl_in[1].gl_Position, gl_in[2].gl_Position)){
        gl_Position = gl_in[1].gl_Position;
        outData.vLineCenter = inData[1].vLineCenter;
        EmitVertex();
        gl_Position = gl_in[2].gl_Position;
        outData.vLineCenter = inData[2].vLineCenter;
        EmitVertex();
        EndPrimitive();
    }
}