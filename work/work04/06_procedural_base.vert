#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// normal position in world coordinates
layout (location = 1) in vec3 normal;

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
// Normal matrix 
uniform mat3 normalMatrix;

uniform vec3 pointLightPosition;

out vec3 lightDir;

out vec3 vNormal;
out vec3 vViewPosition;



void main()
{
	vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1.0f);

	vec4 lightPos = viewMatrix * vec4(pointLightPosition, 1.0); // light position expressed in camera space
	lightDir = normalize(lightPos.xyz - mvPosition.xyz);

	// From camera - vertex position 
	// If we want it in the camera space, camera is the origin -> it is 0. Camera position (0) - mvPosition
	vViewPosition = -mvPosition.xyz;

	vNormal = normalize(normalMatrix*normal);

	// transformations are applied to each vertex
    gl_Position = projectionMatrix * mvPosition;
}