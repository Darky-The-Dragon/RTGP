#version 410 core

const float PI = 3.14159265359;

// output shader variable
out vec4 colorFrag;

in vec3 lightDir;
in vec3 vNormal;
in vec3 vViewPosition;

// force and power of noise
uniform float Kd;
uniform float Ka;
uniform float Ks;
uniform float shininess;
uniform float alpha;
uniform float F0;

uniform vec3 ambientColor;
uniform vec3 specularColor;
uniform vec3 diffuseColor;

// the "type" of the Subroutine
subroutine vec3 ill_model();

// Subroutine Uniform (it is conceptually similar to a C pointer function)
subroutine uniform ill_model Illumination_Model;

//////////////////////////////////////////
// a subroutine for the Lamber model
subroutine(ill_model)
vec3 Lambert() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  // We aply the color if the cosine is greater than 0.
  float lambertian = max(dot(L, N), 0.0);

  return vec3(Kd*lambertian*diffuseColor);
}

// a subroutine for the Phong model
subroutine(ill_model)
vec3 Phong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 color = Ka*ambientColor;

  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  // We aply the color if the cosine is greater than 0.
  float lambertian = max(dot(L, N), 0.0);

  if (lambertian > 0.0) {
    // If it is greater than 0 we need to compute the specular component. We need to calculate the r from the formula
    vec3 V = normalize(vViewPosition);

    vec3 R = reflect(-L, N);

    // Now we caompute the angle between R and V
    float specAngle = max(dot(R,V), 0.0);
    float specular = pow(specAngle, shininess);

    color += vec3 (Kd*lambertian*diffuseColor + Ks*specular*specularColor);
  }

  return color;
}

// a subroutine for the Blinn-Phong model
subroutine(ill_model)
vec3 BlinnPhong() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 color = Ka*ambientColor;

  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  // We aply the color if the cosine is greater than 0.
  float lambertian = max(dot(L, N), 0.0);

  if (lambertian > 0.0) {
    // If it is greater than 0 we need to compute the specular component. We need to calculate the r from the formula
    vec3 V = normalize(vViewPosition);

    vec3 H = normalize(L+V);

    // Now we caompute the angle between R and V
    float specAngle = max(dot(H,V), 0.0);
    float specular = pow(specAngle, shininess);

    color += vec3 (Kd*lambertian*diffuseColor + Ks*specular*specularColor);
  }

  return color;
}

float G1(float angle, float alpha) {
  float r = (alpha+1.0);
  float k = (r*r)/8.0;

  float num = angle;
  float denom = angle * (1.0 - k) + k;

  return num/denom;
}

// a subroutine for the Blinn-Phong model
subroutine(ill_model)
vec3 GGX() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
  vec3 color = Ka*ambientColor;

  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightDir);

  // We aply the color if the cosine is greater than 0.
  float NdotL = max(dot(L, N), 0.0);

  vec3 lambert = (Kd*diffuseColor)/PI;
  vec3 specular = vec3(0.0);

  if (NdotL > 0.0) {
    // If it is greater than 0 we need to compute the specular component. We need to calculate the r from the formula
    vec3 V = normalize(vViewPosition);

    vec3 H = normalize(L+V);

    float NdotH = max(dot(N, H), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    float alpha_Squared = alpha*alpha;
    float NdotH_Squared = NdotH*NdotH;

    vec3 F = vec3(pow(1.0 - VdotH, 5.0));
    F*=(1,0 - F0);
    F+=F0;

    // Now we caclulate the distribution function:
    float D = alpha_Squared;
    float denom = (NdotH_Squared * (alpha_Squared - 1.0) + 1.0);
    D/=PI*denom*denom; 

    // Now we compute the geometry attenuation;
    float G2 = G1(NdotV, alpha) * G1(NdotL, alpha);

    specular = (F*G2*D)/(4.0*NdotV*NdotL);
  }

  return ((lambert+specular)*NdotL);
}

// main
void main(void)
{
  vec3 color = Illumination_Model();
  
  colorFrag = vec4(color, 1.0f);
}
