#version 410

layout (location = 0) in vec3 position;

uniform mat4 modelMatrix;  

uniform mat4 lightSpaceMatrix;

void main()
{
    gl_Position = lightSpaceMatrix * modelMatrix *  vec4(position, 1.0);
}