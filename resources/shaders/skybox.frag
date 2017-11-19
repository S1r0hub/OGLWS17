#version 150

in vec3 eyeDir;

uniform samplerCube Skybox;

out vec4 out_Color;

void main()
{
  out_Color = texture(Skybox, eyeDir);
}
