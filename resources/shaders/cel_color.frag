#version 150

in vec3 color;
in vec3 borderColor;
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 camPos;

// blinn phong shading settings
const vec3 lightPos = vec3(0.0, 0.0, 0.0);
const vec3 specularColor = vec3(1.0, 1.0, 1.0);
const float sunIntensity = 1.0;
const float shininess = 100.0;
const float ambIntensity = 0.00005;
const float diffIntensity = 1.0;
const float specIntensity = 0.8;

// cel shading settings -> maybe make a uniform out of it
const float levels = 4.0;

out vec4 out_Color;


// Returns diffuse and specular multipliers
vec2 shading(vec3 lightDir, vec3 viewDir, float lightInt, float colIntAmb, float colIntDiff, float colIntSpec)
{
  vec3 L = normalize(lightDir);     // normalized light direction
  vec3 V = normalize(viewDir);      // position of the viewer
  vec3 N = normalize(vWorldNormal); // normal vector normalized
  vec3 H = normalize(V + L);        // halfway vector between viewer and light-source

  // the diffuse light factor
  float diffuse = lightInt * colIntDiff * max(0.0, dot(N,L));

  // the specular light factor
  float specular = 0;
  
  if (diffuse > 0.0 && shininess > 0.0)
  {
    float specAngle = max(0.0, dot(N,H));
    specular = colIntSpec * pow(specAngle, shininess);
  }

  // cel shading active if levels are more then zero
  float border = 0.0;
  if (levels > 0.0)
  {
    diffuse = ceil(diffuse * levels) / levels;
    specular = floor(specular * levels) / levels;
  }

  // add ambient color intensity
  diffuse += colIntAmb;

  return vec2(diffuse, specular);
}


void main()
{
  // light and view direction
  vec3 lightDir = lightPos - vWorldPos;
  vec3 viewDir = camPos - vWorldPos;

  // blinn phong shading and cel shading
  vec2 s = shading(lightDir, viewDir, sunIntensity, ambIntensity, diffIntensity, specIntensity);

  // combine the shading with the colors
  vec3 combinedColor = s.x * color +
                       s.y * specularColor;

  // apply a screen gamma
  float screenGamma = 2.2;
  combinedColor = pow(combinedColor, vec3(1.0/screenGamma));

  // use the calculated color for the fragment
  out_Color = vec4(combinedColor, 1.0);
}