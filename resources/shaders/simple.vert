#version 150
#extension GL_ARB_explicit_attrib_location : require
// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 Color;
uniform float EmitValue;

out vec3 pass_Normal;
out vec3 planetColor;
out float planetEmit;
out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 camPos;

void main(void)
{
  pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;
  planetColor = Color;
  planetEmit = EmitValue;

  mat4 iViewMat = inverse(ViewMatrix);
  vec4 mvm = iViewMat[3];
  camPos = mvm.xyz / mvm.w;

  vWorldPos = (ModelMatrix * vec4(in_Position, 1.0 )).xyz;
  vWorldNormal = normalize(vec3(ModelMatrix * vec4(in_Normal, 0.0)));

  gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
}
