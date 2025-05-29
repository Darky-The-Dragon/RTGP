/*
03_normal2color.frag : the Fragment shader assigns to the fragment a color based on the normal vector components

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// output shader variable
out vec4 colorFrag;

// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 N;


void main(){
	// we use the normal as a color
	colorFrag = vec4(normalize(N),1.0);
}
