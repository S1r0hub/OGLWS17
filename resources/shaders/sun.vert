#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
//uniform mat4 NormalMatrix;
uniform vec3 Color;

out vec3 eye;
out vec3 pass_Normal;
out vec3 color;

// texture coordinates
out vec2 texCoord;


void main(void)
{
  vec4 vPosMat = ModelMatrix * vec4(in_Position, 1.0);
  vec3 vWorldPos = vPosMat.xyz / vPosMat.w;
  vec3 vWorldNormal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz;

  // for texturing
  texCoord = in_TexCoord;

  // get camera position from the view matrix
  mat4 ivm = inverse(ViewMatrix);
  vec3 camPos = ivm[3].xyz;

  eye = normalize(camPos - vWorldPos);
  pass_Normal = vWorldNormal;
  color = Color;

	gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}