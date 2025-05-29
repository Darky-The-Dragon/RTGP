/*
02_flatten.frag : Fragment shader, equal to 01_fullcolor.frag

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// output shader variable
out vec4 colorFrag;

// color to assign to the fragments: it is passed from the application
uniform vec3 colorIn;

void main()
{
    colorFrag = vec4(colorIn,1.0);
}
