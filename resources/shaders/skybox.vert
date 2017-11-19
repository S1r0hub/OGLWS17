#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 TexCoords;

void main(void)
{
  // get camera position from the view matrix
  mat4 ivm = inverse(ViewMatrix);
  vec3 camPos = ivm[3].xyz;

  TexCoords = vec3(in_Position.x, -in_Position.yz); // swap skybox because of opengl specification
  gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * vec4(in_Position + camPos, 1.0);
}