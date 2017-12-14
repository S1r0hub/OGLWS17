#version 150

in vec2 TexCoords;

uniform sampler2D frameBufferTex;
uniform int effectFlags;
uniform mat3 Kernel3x3;

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


  // APPLY "PIXEL EFFECTS" HERE:
  if (effectActive(flag_effect_blurred))
  {
    // blurred effect using 3x3 gaussian kernel
    // some nice info: https://en.wikipedia.org/wiki/Kernel_(image_processing)

    color = vec4(0.0);

    // get the pixel size (use original texcoords here!)
    vec2 pixSize = vec2(TexCoords.x / gl_FragCoord.x, TexCoords.y / gl_FragCoord.y);

    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        float kernelWeight = Kernel3x3[x+1][y+1];
        color += texture(frameBufferTex, tc + vec2(x * pixSize.x, y * pixSize.y)) * kernelWeight;
      }
    }
  }


  // APPLY COLOR EFFECTS HERE:

  // apply grayscale effect
  if (effectActive(flag_effect_grayscale))
  {
    float lum = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    color = lum * vec4(1.0, 1.0, 1.0, 1.0);
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
