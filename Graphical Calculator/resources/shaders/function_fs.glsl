#version 330 core

uniform vec3 color;

//https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl
float line_width = 3.0;
float blend_factor = 1.8; //1.5..2.5

in TData{
    vec2 vLineCenter;
} inData;

void main(){
    float distance_from_center = distance(inData.vLineCenter, gl_FragCoord.xy);
    if (distance_from_center>line_width){
        discard;
    }
    gl_FragColor = vec4(color, pow((line_width-distance_from_center)/line_width, blend_factor));
};