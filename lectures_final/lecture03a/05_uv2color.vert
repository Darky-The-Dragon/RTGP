/*
05_uv2color.vert: equal to 00_basic.vert, but UV coordinates are also managed

The UV coordinates of the models are assigned to an oputput variable, so to have them interpolated per-fragment during rasterization

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/


#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// UV texture coordinates
layout (location = 2) in vec2 UV;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// the output variable for UV coordinates
out vec2 interp_UV;

void main()
{
		// I assign the values to a variable with "out" qualifier so to use the per-fragment interpolated values in the Fragment shader
		interp_UV = UV;

		// transformations are applied to each vertex
	   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}
