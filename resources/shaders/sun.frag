#version 150

in vec3 eye;
in vec3 pass_Normal;
in vec3 color;

// for texturing
in vec2 texCoord;
uniform bool useTexture;
uniform sampler2D tex;

const vec3 glowColor = vec3(1.0, 0.4, 0.0);
const float start = 0.1;
const float end = 0.9;
const float alpha = 1.0;

out vec4 out_Color;

// add a glow effect similar to what we did with the diffuse shading
void main()
{
  if (!useTexture)
  {
    vec3 normal = normalize(pass_Normal);
    float rim = smoothstep(start, end, dot(normal, eye));
    float val = clamp(rim, 0.0, 1.0) * alpha;

    vec3 colBase = val * color;
    vec3 colGlow = (1.0 - val) * glowColor;

    out_Color = vec4(colBase + colGlow, 1.0);
  }
  else
  {
    out_Color = texture(tex, texCoord);
  }
}