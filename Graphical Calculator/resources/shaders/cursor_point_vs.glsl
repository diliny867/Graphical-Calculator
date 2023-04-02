#version 330 core

uniform vec2 pos;

void main(){
    gl_Position = vec4(pos, 1.0, 1.0);
}