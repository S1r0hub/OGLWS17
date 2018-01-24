#version 150

// SHADER FOR 3D (IN WORLD) TEXT
// USING THIS SHADER REQUIRES TO ENABLE BLENDING (GL_BLEND)

uniform sampler2D Texture;
uniform vec3 Color;

in vec2 TexCoords;

out vec4 out_Color;

void main()
{
  // use the red value of the texture as alpha -> non-transparent at the characters pixels
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(Texture, TexCoords).r);
  out_Color = vec4(Color, 1.0) * sampled; // apply the color to the "visible" pixels
}
