// APPLICATION SOLAR - Text2D Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)



#ifndef SOLARSYS_TEXT_2DTEXT
#define SOLARSYS_TEXT_2DTEXT


#include <iostream>

// for ortho function
#include <glm/gtc/matrix_transform.hpp>

#include "text_structs.hpp"


// A 2D Text object for on-screen text rendering.
class Text2D
{
  public:

    // the text and integer vector for the color (range 0-255)
    Text2D(std::string text, const Font& font, glm::ivec3 color = glm::ivec3{255}, float winWidth = 800.f, float winHeight = 600.f);

    // render to screen
    void render(GLuint shaderProgram) const;


  private:

    const Font& font_;
    std::string text_;
    glm::ivec3 color_;

    // width and height of the application window
    float winWidth_, winHeight_;

    glm::mat4 projectionMatrix;

    GLuint VAO, VBO;

    // prepare VAO and VBO for rendering
    void prepare();
};

#endif