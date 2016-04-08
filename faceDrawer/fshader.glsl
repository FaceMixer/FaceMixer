#version 150 

//flat in  vec4 color;
in  vec2 texCoord;
out vec4 fColor;
uniform sampler2D texture_s;

void main() 
{
    fColor = texture(texture_s,texCoord);
} 

