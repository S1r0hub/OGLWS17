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

    // the text and integer vector for the color (range 0-1)
    Text2D(std::string text, Font& font, float pos_x, float pos_y, glm::fvec3 color = glm::fvec3{1.0f}, unsigned int winWidth = 800, unsigned int winHeight = 600);

    // render to screen
    void render(GLuint shaderProgram, float scale) const;


  private:

    Font& font_;
    std::string text_;
    glm::ivec3 color_;

    float pos_x_, pos_y_;

    // width and height of the application window
    float winWidth_, winHeight_;

    glm::mat4 projectionMatrix;

    GLuint VAO, VBO;

    // prepare VAO and VBO for rendering
    void prepare();
};

#endif