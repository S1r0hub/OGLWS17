#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 Color;

out vec3 color;
out vec3 borderColor;
out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 camPos;


void main(void)
{
  // pass the colors
  color = Color;

  // get camera position from the view matrix
  mat4 ivm = inverse(ViewMatrix);
  camPos = ivm[3].xyz;

  // get vertex world position and normal world position
  vec4 vPosMat = ModelMatrix * vec4(in_Position, 1.0);
  vWorldPos = vPosMat.xyz / vPosMat.w;
  vWorldNormal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz;

  gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}
