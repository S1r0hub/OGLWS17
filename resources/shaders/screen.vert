#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_TexCoord;

out vec2 TexCoords;

void main(void)
{
  TexCoords = in_TexCoord;
  gl_Position = vec4(in_Position, 1.0);
}