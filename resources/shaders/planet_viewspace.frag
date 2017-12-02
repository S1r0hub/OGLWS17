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

// nightmap
uniform sampler2D tex_night;
const float nightmapIntensity = 0.5;

// normal mapping
uniform sampler2D tex_normal;
uniform float factor_normal;

// specular mapping
//uniform sampler2D tex_specular; // TODO

// texture flags tell which textures are available to use
// for more information about the numbers see "texture_info.hpp"
uniform int texture_flags;
const int flag_tex_normal = 1;
const int flag_tex_specular = 2;
const int flag_tex_night = 4;


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



// Determine if the flag is active or not.
// Returns if the texture is available.
bool hasTex(int flag_tex)
{
  return bool(texture_flags & flag_tex);
}



void main()
{
  // light and view direction
  vec3 lightDir = lightPos - vViewPos;
  vec3 viewDir = -vViewPos;
  vec3 normal = vViewNormal;



  // ===== normal mapping ===== //

  if (hasTex(flag_tex_normal))
  {
    // Restore normal from normal map texture
    vec3 normalFromTex = normalize(texture(tex_normal, texCoord).xyz * 2.0 - 1.0);

    // use the factor of normal map
    vec3 N = normalize(normalFromTex * factor_normal + vec3(0.0, 0.0, 1.0) * (1.0 - factor_normal));

    lightDir = TBN * normalize(lightDir);
    viewDir = TBN * normalize(viewDir);
    normal = N;

    // debugging
    /*
    if (bool(texture_flags & texFlag_normal))
    {
      out_Color = vec4(normal, 1.0);
      return;
    }
    */
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

    if (hasTex(flag_tex_night))
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