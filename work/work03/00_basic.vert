/*
00_basic.vert : basic Vertex shader

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2022/2023
Master degree in Computer Science
Universita' degli Studi di Milano

*/


#version 410 core

// vertex position in world coordinates
// the number used for the location in the layout qualifier is the position of the vertex attribute
// as defined in the Mesh class
layout (location = 0) in vec3 position;
//layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 UV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

//uniform mat3 normalMatrix;

// uniform float timer;
// uniform float weight;

//out vec3 N;

out vec2 interp_UV;

void main()
{
    vec3 flattened = position;
    flattened.z = 0;

    //N = normalize(normalMatrix * normal);

    interp_UV = UV;

    // I am taking a vertex and move it up and down
    // float disp = weight * sin(timer) + weight;
    // vec3 newPos = position + disp * normal; 

    // we assign the original vertex position in output
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
    // vec4(newPos, 1.0f);
    // vec4(position, 1.0f);
}
