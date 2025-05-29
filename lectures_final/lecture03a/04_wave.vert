/*
04_wave.vert: the Vertex shader applies a cyclic deformation to the vertices, using the sine function applied to the execution time of the application

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal in world coordinate
layout (location = 1) in vec3 normal;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;

// normals transformation matrix (= transpose of the inverse of the model-view matrix)
uniform mat3 normalMatrix;

// the execution time of the application is passed to the shader using an uniform variable
uniform float timer;

// the application passes usign an uniform also the weight to apply to the displacement
uniform float weight;

// the transformed normal is set as an output variable, to be "passed" to the fragment shader
// this means that the normal values in each vertex will be interpolated on each fragment created during rasterization between two vertices
out vec3 N;


void main()
{
		// we modify the original position of the vertex,
		// applying a displacement based on the execution time of the application
		// The result is an alternate "wobble "effect

		// displacement is based on the execution time, weighted and modified by the weight parameter
		float disp = weight*sin(timer)+weight;
		// the displacement is applied along the normal direction
		vec3 newPos = position + disp*normal;

		// transformations are applied to the new vertex
	  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(newPos, 1.0f);
		// transformations are applied to the normal
	  N = normalize(normalMatrix * normal);
}
