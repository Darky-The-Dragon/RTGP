#version 410 core

layout (location = 0 ) in vec3 position;

void main()
{
    // We cas position to vec4
    // gl_Position = vec4(position.x, position.y, position.z, 1.0); Equivalent to:
    gl_Position = vec4(position, 1.0);
}