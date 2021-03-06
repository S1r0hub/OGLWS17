#version 150
#extension GL_ARB_explicit_attrib_location : require

// vertex attributes of VAO
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;
layout(location = 3) in vec3 in_Tangent;


//Matrix Uniforms as specified with glUniformMatrix4fv
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 NormalMatrix;
uniform vec3 Color;


//out vec3 pass_Normal;
out vec3 planetColor;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 camPos;

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


  // calculate camera position (another way)
  /*
  mat4 mvm = ViewMatrix;
  mat3 rotMat = mat3(mvm[0].xyz, mvm[1].xyz, mvm[2].xyz);
  vec3 d = mvm[3].xyz;
  camPos = -d * rotMat;
  */


  // get camera position from the view matrix
  mat4 ivm = inverse(ViewMatrix);
  camPos = ivm[3].xyz;


  // get vertex world position and normal world position
  vec4 vPosMat = ModelMatrix * vec4(in_Position, 1.0);
  vWorldPos = vPosMat.xyz / vPosMat.w;
  vWorldNormal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz;

  gl_Position = (ProjectionMatrix  * ViewMatrix * ModelMatrix) * vec4(in_Position, 1.0);



  // FOR NORMAL MAPPING

  // Transform Normal and Tangent to world space
  vec3 modelNormal = (ModelMatrix * vec4(in_Normal, 0.0)).xyz; //(NormalMatrix * vec4(in_Normal, 0.0)).xyz;
  vec3 modelTangent = (ModelMatrix * vec4(in_Tangent, 0.0)).xyz;

  // Bitangent Calculation
  vec3 modelBitangent = cross(modelNormal, modelTangent);

  TBN = transpose(mat3(modelTangent, modelBitangent, modelNormal));
}
