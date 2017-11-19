#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

out vec3 eyeDir;

// infos here https://gamedev.stackexchange.com/questions/60313/implementing-a-skybox-with-glsl-version-330
void main(void)
{
  mat4 inverseProjection = inverse(ProjectionMatrix);
  mat3 invModelView = mat3(inverse(ViewMatrix * ModelMatrix));
  vec3 unProj = (inverseProjection * vec4(in_Position, 1.0)).xyz;
  eyeDir = invModelView * unProj;

  gl_Position = vec4(in_Position, 1.0);
}