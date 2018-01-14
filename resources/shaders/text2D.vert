#version 150
#extension GL_ARB_explicit_attrib_location : require

// SHADER FOR 2D (ON SCREEN) TEXT

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;

uniform mat4 ProjectionMatrix;

out vec2 TexCoords;

void main()
{
  TexCoords = in_TexCoords;
  gl_Position = ProjectionMatrix * vec4(in_Position, 0.0, 1.0);
}
