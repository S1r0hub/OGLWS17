#version 150

in vec3 pass_Normal;
in vec3 planetColor;
in float planetEmit;

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 camPos;

// position of sun
const vec3 lightPos = vec3(0.0, 0.0, 0.0);
const vec3 ambientColor = vec3(0.0, 0.0, 0.0);
const vec3 specularColor = vec3(1.0, 1.0, 1.0);
const float shininess = 1.0;

out vec4 out_Color;

void main()
{
  vec3 lightDir = normalize(lightPos - vWorldPos);

  float brightness = max(dot(vWorldNormal, lightDir), 0.0);

  vec3 viewDir = normalize(camPos);
  vec3 halfDir = normalize(lightDir + viewDir);
  float specAngle = max(dot(halfDir, pass_Normal), 0.0);
  float specular = pow(specAngle, shininess) / 8.0;

  vec3 combinedColor = ambientColor +
                       planetEmit * planetColor +
                       brightness * planetColor +
                       specular * planetColor;

  out_Color = vec4(combinedColor, 1.0);
}