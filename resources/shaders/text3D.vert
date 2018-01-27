#version 150
#extension GL_ARB_explicit_attrib_location : require

// SHADER FOR 3D (IN WORLD) TEXT

layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec2 in_TexCoords;


uniform mat4 ModelMatrix;

layout (std140) uniform camera_data
{
  mat4 ViewMatrix;
  mat4 ProjectionMatrix;
};


out vec2 TexCoords;

// Sprite face cam: https://stackoverflow.com/questions/5763577/how-can-i-draw-an-object-in-opengl-that-always-faces-the-camera

void main()
{
  TexCoords = in_TexCoords;


  // The following will clear the rotational part of the transformation
  // OpenGL defaults to column major matrices -> access is of the format mat[col][row]
  mat4 mat = ViewMatrix * ModelMatrix;
  float d = sqrt(pow(mat[0][0],2) + pow(mat[1][1],2) + pow(mat[2][2],2));
  mat4 matNew = mat4(d); // create a diagonal matrix using the calculated scale factor
  matNew[3][3] = 1;
  matNew[3][0] = mat[3][0];
  matNew[3][1] = mat[3][1];
  matNew[3][2] = mat[3][2];


  gl_Position = (ProjectionMatrix * matNew) * vec4(in_Position, 0.0, 1.0);
}
