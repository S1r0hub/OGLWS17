#version 150

in  vec3 obitColor;
out vec4 out_Color;

void main()
{
  out_Color = vec4(normalize(obitColor), 1.0);
}
