#version 150

in vec2 TexCoords;

uniform sampler2D frameBufferTex;
uniform int effectFlags;

const int flag_effect_grayscale = 1;
const int flag_effect_mirrored_horizontal = 2;
const int flag_effect_mirrored_vertical = 4;
const int flag_effect_blurred = 8;

out vec4 out_Color;


bool effectActive(int effectFlag)
{
  return bool(effectFlags & effectFlag);
}


// apply active effects
vec4 applyEffects(vec4 color)
{
  vec2 tc = TexCoords;

  // horizontal mirrored texture coordinates
  if (effectActive(flag_effect_mirrored_horizontal))
  { tc.x = 1.0 - tc.x; }

  // vertical mirrored texture coordinates
  if (effectActive(flag_effect_mirrored_vertical))
  { tc.y = 1.0 - tc.y; }

  // apply mirroring
  color = texture(frameBufferTex, tc);


  // APPLY COLOR EFFECTS HERE:

  // apply grayscale effect
  if (effectActive(flag_effect_grayscale))
  {
    float lum = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = lum * vec4(1.0, 1.0, 1.0, 1.0);
  }


  // APPLY "PIXEL EFFECTS" HERE:
  if (effectActive(flag_effect_blurred))
  {
    // TODO: implement blurred effect using 3x3 gaussian kernel
  }

  return color;
}


void main()
{
  // get the color from the framebuffer texture, apply effects and return it
  vec4 color = texture(frameBufferTex, TexCoords);
  color = applyEffects(color);
  out_Color = color;
}
