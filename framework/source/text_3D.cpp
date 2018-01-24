// APPLICATION SOLAR - Text3D Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)

#include "text_3D.hpp"


Text3D::Text3D(std::string text, std::shared_ptr<Font> font, glm::fvec3 pos, glm::fvec3 color, unsigned int winWidth, unsigned int winHeight)
  : Text(text, font, pos, color, winWidth, winHeight)
{
  // ensure color is in range of 0 to 1
  color = glm::normalize(color);

  // prepare VBO, VAO...
  prepare();
}


void Text3D::setModelMatrix(glm::mat4& modelMatrix)
{
  modelMatrix_ = modelMatrix;
}


void Text3D::setPosition(float x, float y, float z)
{
  Text::setPosition(glm::fvec3{x,y,z});
}


void Text3D::prepare()
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
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

  // "unbind" the buffer (not needed usually but done to ensure no later changes)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Text3D::render(GLuint shaderProgram, float scale) const
{
  //glEnable(GL_CULL_FACE);
  glDepthMask(GL_FALSE);

  // enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // use the desired shader program
  glUseProgram(shaderProgram);

  // upload text model matrix
  GLuint uniformLoc = glGetUniformLocation(shaderProgram, "ModelMatrix");
  glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix_));

  // upload text color
  uniformLoc = glGetUniformLocation(shaderProgram, "Color");
  glUniform3f(uniformLoc, color_.r, color_.g, color_.b);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);

  float x = pos_.x;
  float y = pos_.y;

  // for all characters of the string...
  for (std::string::const_iterator c = text_.begin(); c != text_.end(); ++c)
  {
    TextCharacter tc;
    try { tc = font_->characters.at(*c); }
    catch (...)
    {
      std::cerr << "Failed to render a character (" << *c << ")!" << std::endl;
      continue;
    }

    // x and y position of the character
    GLfloat xpos = x + tc.bearing.x * scale;
    GLfloat ypos = y - (tc.dimension.y - tc.bearing.y) * scale;

    // width and height of the character
    GLfloat w = tc.dimension.x * scale;
    GLfloat h = tc.dimension.y * scale;

    // update the VBO for each character
    GLfloat vertices[6][4] =
    {
      { xpos,     ypos + h,   0.0, 0.0 },
      { xpos,     ypos,       0.0, 1.0 },
      { xpos + w, ypos,       1.0, 1.0 },

      { xpos,     ypos + h,   0.0, 0.0 },
      { xpos + w, ypos,       1.0, 1.0 },
      { xpos + w, ypos + h,   1.0, 0.0 }
    };

    // render the specific glyph texture over the quad
    // using the texture we loaded by initializing the font
    glBindTexture(GL_TEXTURE_2D, tc.textureID);

    // update content of VBO by uploading the vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // render the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // the "advance cursors" for next glyph (advance is number of 1/64 pixels)
    x += (tc.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
  }

  // disable BLEND (because we did activate it at start)
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}
