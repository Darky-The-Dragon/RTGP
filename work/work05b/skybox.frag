#version 410 core
out vec4 colorFrag;

in vec3 interp_UVW;

uniform samplerCube tCube;

void main()
{
    colorFrag = texture(tCube, interp_UVW);
}