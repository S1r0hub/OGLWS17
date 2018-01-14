// APPLICATION SOLAR - Text2D Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)

#include "text_2D.hpp"


Text2D::Text2D(std::string text, const Font& font, glm::ivec3 color, float winWidth, float winHeight)
  : text_(text)
  , font_(font)
  , color_(color)
  , winWidth_(winWidth)
  , winHeight_(winHeight)
{
  // create orthogonal projection matrix (for 2D view)
  projectionMatrix = glm::ortho(0.0f, winWidth, 0.0f, winHeight);
}


void Text2D::prepare()
{
  // create vertex array object (VAO) and vertex buffer object (VBO)
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // The quad requires: 2 triangles => 6 vertices with each: position (vec2) + texcoord (vec2) => 4 floats
  // GL_DYNAMIC_DRAW because data updates occur many times
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

  // location 0 in shader (= in_Position / 2D Vertex Position)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

  // location 1 in shader (= in_TexCoords / 2D Texture Position)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2* sizeof(GLfloat)));
  
  // "unbind" the buffer (not needed usually but done to ensure no later changes)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  //glBindVertexArray(0); // no need because we will quickly change anyway
}


void Text2D::render(GLuint shaderProgram) const
{
  // enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // use the desired shader program
  glUseProgram(shaderProgram);

  // upload text color
  GLuint uniformLoc = glGetUniformLocation(shaderProgram, "Color");
  glUniform3f(uniformLoc, color_.r, color_.g, color_.b);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  // for all characters of the string...
  std::string::const_iterator c;
  for (c = text_.begin(); c != text_.end(); c++);
  {
    bool err = false;
    TextCharacter tc;
    try { tc = font_.characters.at(*c); }
    catch (const std::out_of_range)
    {
      std::cerr << "Failed to render a character (" << *c << ")!" << std::endl;
      err = true;
    }

    //if (err) { continue; }
     
    // TODO: continue here!!
  }
}
