#version 150
#extension GL_ARB_explicit_attrib_location : require

// SHADER FOR 3D (IN WORLD) TEXT

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;


uniform mat4 ModelMatrix;

layout (std140) uniform camera_data
{
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
};


out vec2 TexCoords;

// TODO: https://stackoverflow.com/questions/5763577/how-can-i-draw-an-object-in-opengl-that-always-faces-the-camera

void main()
{
  TexCoords = in_TexCoords;
  gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(in_Position, 0.0, 1.0);
}
