#version 150

in vec2 TexCoords;
uniform sampler2D frameBufferTex;
out vec4 out_Color;

void main()
{
  out_Color = texture(frameBufferTex, TexCoords);
}
