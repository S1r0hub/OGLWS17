#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 BorderColor;

// TODO: to be added as uniform
const float borderThickness = 0.02;

out vec3 color;

void main(void)
{
  vec3 posOut = in_Position + in_Normal * borderThickness;
  color = BorderColor;
	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(posOut, 1.0);
}