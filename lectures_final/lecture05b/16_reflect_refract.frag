/*
16_reflect_refract.frag: Fragment shaders for reflections and Fresnel equations

N.B. 1)  "15_reflect_refract.vert" must be used as vertex shader

N.B. 2)  the different effects are implemented using Shaders Subroutines

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

// vertex position and normal in world coordinates
in vec4 worldPosition;
in vec3 worldNormal;

// texture sampler: we are using a cube map, so we need a samplerCube
uniform samplerCube tCube;

// camera position
uniform vec3 cameraPosition;

// point light position
uniform vec3 pointLightPosition;

// Eta (= ratio between the 2 refraction indices)
uniform float Eta;
// exponent of Fresnel equation (= 5 in literature, but we can change its value to have more artistic control on the final effect)
uniform float mFresnelPower;


////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec4 refl_refra();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform refl_refra Reflection_Refraction;

////////////////////////////////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for the reflection using an environment map
subroutine(refl_refra)
vec4 Reflection() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // vector from vertex to camera in world coordinates
    vec3 V = normalize(worldPosition.xyz - cameraPosition);

    // reflection vector of I with respect to normal
    vec3 R = reflect( V, normalize(worldNormal));

    // we sample the texture cube using the components of the reflection vector as texture coordinates.
    return texture( tCube, R);
}
//////////////////////////////////////////

//////////////////////////////////////////
// a subroutine for refraction using Fresnel
subroutine(refl_refra)
vec4 Fresnel() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    vec3 N = normalize(worldNormal);

    // vector from vertex to camera in world coordinates
    vec3 V = normalize(worldPosition.xyz - cameraPosition);

    // incidence light direction in world coordinates
    vec3 L = normalize(worldPosition.xyz - pointLightPosition);

    // half vector in world coordinates
    vec3 H = normalize(L + V);

    // reflection vector of V with respect to normal
    vec3 R = reflect(V, N);

    // we sample the texture cube using the components of the reflection vector as texture coordinates.
    vec4 reflectedColor = texture( tCube, R);

    // refraction vectors
    // we use 3 vectors, one for each RGB channel, to simulate chromatic aberration:
    // we use a slightly different value of Eta in the second and third vector, in order to have 3 slightly different colors
    vec3 refractDir[3];
    refractDir[0] = refract( V , N, Eta );
    refractDir[1] = refract( V , N, Eta * 0.99 );
    refractDir[2] = refract( V , N, Eta * 0.98 );

    // we sample 3 times the cube map for the refraction, and we use only 1 channel for each for each sample
    vec4 refractedColor = vec4( 1.0 );
    refractedColor.r = texture( tCube,refractDir[0]).r;
    refractedColor.g = texture( tCube,refractDir[1]).g;
    refractedColor.b = texture( tCube,refractDir[2]).b;

    // Fresnel equation with Schlik's approximation
    // F(0°) factor
    float F0 = ((1.0-Eta)*(1.0-Eta)) / ((1.0+Eta)*(1.0+Eta));
    // ratio between reflection and refraction
    float Ratio = F0 + (1.0-F0) * pow( 1.0 - max(dot(V, H),0.0), mFresnelPower );

    // we merge the 2 colors, using the ratio calculated with the Fresnel equation
    return mix( refractedColor, reflectedColor, clamp( Ratio, 0.0, 1.0 ));
}
//////////////////////////////////////////

// main
void main(void)
{
    // we call the pointer function Reflection_Refraction():
    // the subroutine selected in the main application will be called and executed
    colorFrag = Reflection_Refraction();
}
