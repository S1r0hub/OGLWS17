#version 150

//in vec3 pass_Normal;
in vec3 planetColor;

in vec3 lightPos;
in vec3 vViewPos;
in vec3 vViewNormal;

in mat3 TBN;


// for texturing
in vec2 texCoord;
uniform bool useTexture;
uniform sampler2D tex;

uniform sampler2D tex_night;          // nightmap - e.g. for earth
uniform bool hasTex_night;            // if the night texture is available
const float nightmapIntensity = 0.5;  // strength of the nightmap

uniform sampler2D tex_normal;     // normal map texture
uniform bool hasTex_normal;


// blinn phong shading settings
const vec3 specularColor = vec3(1.0, 1.0, 1.0);
const float sunIntensity = 1.0;
const float shininess = 80.0;
const float ambIntensity = 0.01;
const float diffIntensity = 1.0;
const float specIntensity = 0.5;

out vec4 out_Color;


// Returns diffuse and specular multipliers
vec2 blinnPhong(vec3 lightDir, vec3 viewDir, vec3 normal, float lightInt, float colIntAmb, float colIntDiff, float colIntSpec)
{
  vec3 L = normalize(lightDir); // normalized light direction
  vec3 V = normalize(viewDir);  // position of the viewer
  vec3 N = normalize(normal);   // normal vector normalized
  vec3 H = normalize(V + L);    // halfway vector between viewer and light-source

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
  vec3 lightDir = lightPos - vViewPos;
  vec3 viewDir = -vViewPos;
  vec3 normal = vViewNormal;



  // ===== normal mapping stuff ===== //

  if (hasTex_normal)
  {
    // Restore normal from normal map texture
    vec3 normalFromTex = normalize(texture(tex_normal, texCoord).xyz * 2.0 - 1.0);

    // Decrease details (mixing normal with uniform normal)
    float factor = 5.0;
    vec3 N = normalize(normalFromTex * factor + vec3(0.0, 0.0, 1.0) * (1.0 - factor));

    lightDir = TBN * normalize(lightDir);
    viewDir = TBN * normalize(viewDir);
    normal = N;
  }

  // ================================ //



  // blinn phong shading
  vec2 bps = blinnPhong(lightDir, viewDir, normal, sunIntensity, ambIntensity, diffIntensity, specIntensity);


  // combine the shading with the colors
  vec3 combinedColor = bps.x * planetColor +
                       bps.y * specularColor;


  vec4 finalColor = vec4(combinedColor, 1.0);

  // apply the texture instead of the default diffuse color
  if (useTexture)
  {
    // texture only
    vec3 white = vec3(1.0, 1.0, 1.0);
    finalColor = texture(tex, texCoord) * vec4((bps.x * white), 1.0);

    if (hasTex_night)
    {
      float rim = clamp(smoothstep(0.8, 1.1, (1.0 - bps.x)), 0.0, 1.0);
      finalColor += texture(tex_night, texCoord) * vec4((rim * white * nightmapIntensity), 1.0);
    }

    // texture mixed with planet color
    //finalColor = texture(tex, texCoord) * finalColor;
  }


  // use the calculated color for the fragment
  out_Color = vec4(finalColor);
}