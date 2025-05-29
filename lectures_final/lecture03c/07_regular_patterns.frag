/*
07_regular_patterns.frag: a fragment shader with procedural regular patterns (stripled and circles) with and without aliasing

N.B. 1)  "06_procedural_base.vert" must be used as vertex shader

N.B. 2)  the different effects are implemented using Shaders Subroutines

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// output shader variable
out vec4 colorFrag;

// UV texture coordinates, interpolated in each fragment by the rasterization process
in vec2 interp_UV;

// texture repetitions
uniform float repeat;

// stripes colors
uniform vec3 color1;
uniform vec3 color2;

////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

// aastep function calculates the length of the gradient given from the difference between the current fragment and the neighbours on the right and on the top. We can then apply a smoothstep function using as threshold the value given by the gradient.
float aastep(float threshold, float value) {
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));

  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine float reg_patterns();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform reg_patterns Regular_Patterns;

//////////////////////////////////////////
// a subroutine for a stripes pattern effect (with aliasing)
subroutine(reg_patterns)
float Stripes() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // s coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    float k = fract(interp_UV.s * repeat);
    // from [0.0,1.0] to [-1.0,1.0] -> 2.0*k -1.0
    // taking the absolute value, we "translate" the pattern to the left
    // -> try to save in f the value of k to see what happens to the colors mix
    k = abs(2.0*k -1.0);
    // step function forces half pattern to white and the other half to black
    // we use this value to mix the colors, to obtain a colored stripes pattern
    // A LOT OF ALIASING!
    return step(0.5,k);
}

//////////////////////////////////////////
// a subroutine for a stripes pattern effect (with antialiasing based on smoothstep)
subroutine(reg_patterns)
float StripesSmoothstep() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  // s coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    float k = fract(interp_UV.s * repeat);
    // from [0.0,1.0] to [-1.0,1.0] -> 2.0*k -1.0
    // taking the absolute value, we "translate" the pattern to the left
    // -> try to save in f the value of k to see what happens to the colors mix
    k = abs(2.0*k -1.0);
    // step function forces half pattern to white and the other half to black
    // we use smoothstep to create the stripes, with a small gradient to apply antialising
    // we use this value to mix the colors, to obtain a colored stripes pattern
    // LESS ALIASING THAN BEFORE, BUT STILL EVIDENT
    return smoothstep(0.45,0.5,k);
}

//////////////////////////////////////////
// a subroutine for a stripes pattern effect (with antialiasing based on aastep)
subroutine(reg_patterns)
float StripesAAstep() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  // s coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    float k = fract(interp_UV.s * repeat);
    // from [0.0,1.0] to [-1.0,1.0] -> 2.0*k -1.0
    // taking the absolute value, we "translate" the pattern to the left
    // -> try to save in f the value of k to see what happens to the colors mix
    k = abs(2.0*k -1.0);
    // step function forces half pattern to white and the other half to black
    // we use aasteo to create the stripes using adaptive antialiasing
    // we use this value to mix the colors, to obtain a colored stripes pattern
    // ALIASING ELIMINATED!
    return aastep(0.5,k);
}

//////////////////////////////////////////
// a subroutine for a circles pattern effect (with aliasing)
subroutine(reg_patterns)
float Circles() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // UV coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    // in this case, we apply the procedure to both the texture coordinates, generating a checkboard of gradients
    vec2 k = fract(interp_UV * repeat);
    // k -> from 0.0 to 1.0
    // we transform it to have a value from -0.5 to 0.5, and we take the vector length
    // we obtain a radial gradient in each checkboard position
    // applying the step function, we isolate a circular pattern
    // we use this value to mix the colors, to obtain a colored circle pattern
    // A LOT OF ALIASING!
    return step(0.3, length(k-0.5));
}

//////////////////////////////////////////
// a subroutine for a circles pattern effect (with antialiasing based on smoothstep)
subroutine(reg_patterns)
float CirclesSmoothstep() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // UV coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    // in this case, we apply the procedure to both the texture coordinates, generating a checkboard of gradients
    vec2 k = fract(interp_UV * repeat);
    // k -> from 0.0 to 1.0
    // we transform it to have a value from -0.5 to 0.5, and we take the vector length
    // we obtain a radial gradient in each checkboard position
    // we use smoothstep to create the circles, with a small gradient to apply antialising
    // we use this value to mix the colors, to obtain a colored circle pattern
    // LESS ALIASING THAN BEFORE, BUT STILL EVIDENT
    return smoothstep(0.3, 0.32, length(k-0.5));
}

//////////////////////////////////////////
// a subroutine for a circles pattern effect (with antialiasing based on aastep)
subroutine(reg_patterns)
float CirclesAAstep() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // UV coordinates -> from 0.0 to 1.0
    // multiplied for "repeat" -> from 0.0 to "repeat"
    // fractional part gives the repetition of a gradient from 0.0 to 1.0 for "repeat" times
    // in this case, we apply the procedure to both the texture coordinates, generating a checkboard of gradients
    vec2 k = fract(interp_UV * repeat);
    // k -> from 0.0 to 1.0
    // we transform it to have a value from -0.5 to 0.5, and we take the vector length
    // we obtain a radial gradient in each checkboard position
    // we use aastep to create the circles using adaptive antialiasing
    // we use this value to mix the colors, to obtain a colored circle pattern
    // ALIASING ELIMINATED!
    return aastep(0.3, length(k-0.5));
}
///////////////////////////////////////////////////

// main
void main(void)
{
    // we call the pointer function Regular_Patterns():
    // the subroutine selected in the main application will be called and executed
    float f = Regular_Patterns();

    // we use f to mix the colors, to obtain a colored regular pattern
  	colorFrag = vec4(mix(color1, color2, f),1.0);
}
