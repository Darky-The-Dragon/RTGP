/*
10_illumination_models.frag: Fragment shader for the Lambert, Phong, Blinn-Phong and GGX illumination models

N.B. 1)  "09_illumination_models.vert" must be used as vertex shader

N.B. 2)  the different illumination models are implemented using Shaders Subroutines

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

// light incidence direction (calculated in vertex shader, interpolated by rasterization)
in vec4 worldPosition;
// the transformed normal has been calculated per-vertex in the vertex shader
in vec3 worldNormal;

uniform samplerCube tCube;

uniform vec3 pointLightPosition;
uniform vec3 cameraPosition;

uniform float Eta; 
uniform float mFresnelPower;

////////////////////////////////////////////////////////////////////

// the "type" of the Subroutine
subroutine vec4 ill_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

//////////////////////////////////////////
// a subroutine for the Blinn-Phong model
subroutine(ill_model)
vec4 Reflect() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    vec3 N = normalize(worldNormal);
    
    vec3 V = normalize(worldPosition.xyz - cameraPosition.xyz);

    vec3 R = normalize(reflect(V,N));

    return texture(tCube, R);
}
//////////////////////////////////////////

subroutine(ill_model)
vec4 Fresnel()
{  
    vec3 N = normalize(worldNormal);
    vec3 V = normalize(worldPosition.xyz - cameraPosition.xyz);

    vec3 L = normalize(worldPosition.xyz - pointLightPosition);

    vec3 H = normalize(L + V);

    vec3 R = reflect(V, N);

    vec4 reflectedColor = texture(tCube, R);

    vec3 refractDir[3];
    refractDir[0] = refract(V, N, Eta);
    refractDir[1] = refract(V, N, Eta * 0.99);
    refractDir[2] = refract(V, N, Eta * 0.98);

    vec4 refractedColor = vec4(1.0);
    refractedColor.r = texture(tCube, refractDir[0]).r;
    refractedColor.g = texture(tCube, refractDir[1]).g;
    refractedColor.b = texture(tCube, refractDir[2]).b;

    float F0 = ((1.0 - Eta) * (1.0 - Eta)) / ((1.0 + Eta) * (1.0 + Eta));
    float Ratio = F0 + (1.0 - F0) * pow(1.0 - max(dot(V,H), 0.0), mFresnelPower);

    return mix(refractedColor, reflectedColor, clamp(Ratio, 0.0, 1.0));
}

// main
void main(void)
{
    colorFrag = Illumination_Model();
}
