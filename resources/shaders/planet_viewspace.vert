#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in vec3 in_Tangent;


//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;

layout (std140) uniform camera_data
{
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
};

uniform mat4 NormalMatrix;
uniform vec3 Color;


const vec3 lightPosConst = vec3(0.0, 0.0, 0.0);


//out vec3 pass_Normal;
out vec3 planetColor;

out vec3 lightPos;
out vec3 vViewPos;
out vec3 vViewNormal;
//out vec3 camPos;

// texture coordinates
out vec2 texCoord;

// Tangent, Bitangent, Normal Matrix
out mat3 TBN;


void main(void)
{
  //pass_Normal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;
  planetColor = Color;

  // for texturing
  texCoord = in_TexCoord;

  // pass light pos
  lightPos = (ViewMatrix * vec4(lightPosConst, 1.0)).xyz;


  // get vertex world position and normal world position
  vec4 vPosMat = (ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);
  vViewPos = vPosMat.xyz / vPosMat.w;
  vViewNormal = (NormalMatrix * vec4(in_Normal, 0.0)).xyz;

  gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);


  // FOR NORMAL MAPPING

  // Transform Tangent to view space
  vec3 modelTangent = (NormalMatrix * vec4(in_Tangent, 0.0)).xyz;

  // Bitangent Calculation
  vec3 modelBitangent = (NormalMatrix * vec4(cross(in_Normal, in_Tangent), 1.0)).xyz;

  TBN = transpose(mat3(modelTangent, modelBitangent, vViewNormal));
}
