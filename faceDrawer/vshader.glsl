#version 150 

in vec4 vPosition;
in vec2 s_vTexCoord;
out vec2 texCoord;

uniform mat4 model_view;
uniform mat4 projection;

void main() 
{
    gl_Position =  model_view * vPosition;
    texCoord = s_vTexCoord;
} 
