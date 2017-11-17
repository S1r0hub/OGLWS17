#version 150

//in vec3 pass_Normal;
in vec3 planetColor;

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 camPos;


// blinn phong shading settings
const vec3 lightPos = vec3(0.0, 0.0, 0.0);
const vec3 specularColor = vec3(1.0, 1.0, 1.0);
const float sunIntensity = 1.0;
const float shininess = 80.0;
const float ambIntensity = 0.00005;
const float diffIntensity = 1.0;
const float specIntensity = 0.5;

out vec4 out_Color;


// Returns diffuse and specular multipliers
vec2 blinnPhong(vec3 lightDir, vec3 viewDir, float lightInt, float colIntAmb, float colIntDiff, float colIntSpec)
{
  vec3 L = normalize(lightDir);     // normalized light direction
  vec3 V = normalize(viewDir);      // position of the viewer
  vec3 N = normalize(vWorldNormal); // normal vector normalized
  vec3 H = normalize(V + L);        // halfway vector between viewer and light-source

  // the diffuse light factor
  //float diffuse = lightInt * colIntDiff * max(0.0, dot(N,L));
  float diffuse = lightInt * colIntDiff * clamp(dot(N,L), 0, 1);

  // the specular light factor
  float specular = 0;
  
  if (diffuse > 0.0 && shininess > 0.0)
  {
    float specAngle = max(0.0, dot(N,H));
    specular = colIntSpec * pow(specAngle, shininess);
  }

  // add ambient light factor to the diffuse light factor
  diffuse += colIntAmb;

  return vec2(diffuse, specular);
}


void main()
{
  // light and view direction
  vec3 lightDir = lightPos - vWorldPos;
  vec3 viewDir = camPos - vWorldPos;

  // blinn phong shading
  vec2 bps = blinnPhong(lightDir, viewDir, sunIntensity, ambIntensity, diffIntensity, specIntensity);

  // combine the shading with the colors
  vec3 combinedColor = bps.x * planetColor +
                       bps.y * specularColor;

  // apply a screen gamma
  float screenGamma = 2.2;
  combinedColor = pow(combinedColor, vec3(1.0/screenGamma));

  // use the calculated color for the fragment
  out_Color = vec4(combinedColor, 1.0);
}