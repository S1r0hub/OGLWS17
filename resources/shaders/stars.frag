#version 150

in  vec3 color_star;
out vec4 out_Color;

void main()
{
  out_Color = vec4(normalize(color_star), 1.0);
}
