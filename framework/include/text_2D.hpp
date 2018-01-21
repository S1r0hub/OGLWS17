// APPLICATION SOLAR - Text2D Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)



#ifndef SOLARSYS_TEXT_2DTEXT
#define SOLARSYS_TEXT_2DTEXT

#include "text.hpp"


// A 2D Text object for on-screen (GUI) text rendering.
class Text2D : public Text
{
  public:

    // the text and integer vector for the color (range 0-1)
    Text2D(std::string text, Font& font, glm::fvec2 pos, glm::fvec3 color = glm::fvec3{1.0f}, unsigned int winWidth = 800, unsigned int winHeight = 600);

    // change the location of the text
    void setPosition(float x, float y);

    // render to screen
    void render(GLuint shaderProgram, float scale) const override;


  private:

    glm::mat4 projectionMatrix;

    // prepare VAO and VBO for rendering
    void prepare() override;
};

#endif
