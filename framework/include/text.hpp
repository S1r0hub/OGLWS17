// APPLICATION SOLAR - Text Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)



#ifndef SOLARSYS_TEXT_TEXT
#define SOLARSYS_TEXT_TEXT


#include <iostream>

// for ortho function
#include <glm/gtc/matrix_transform.hpp>
#include <memory> // included to use shared pointers (std::shared_ptr)

#include "text_structs.hpp"


// Base class for text
class Text
{
  public:

    // the text and integer vector for the color (range 0-1)
    Text(std::string text, std::shared_ptr<Font> font, glm::fvec3 pos, glm::fvec3 color = glm::fvec3{1.0f}, unsigned int winWidth = 800, unsigned int winHeight = 600);


    // #### GETTER #### //

    // get the text position
    virtual glm::fvec3 getPosition();

    // get the color of the text
    virtual glm::fvec3 getColor();

    // get the string of this text
    virtual std::string getText();


    // #### SETTER #### //

    // change the currently shown text
    virtual void setText(std::string text);

    // set the text font
    void setFont(std::shared_ptr<Font> font);

    // change the location of the text
    virtual void setPosition(glm::fvec3 pos);

    // change the location of the text
    void setPosition(float x = 1.f, float y = 1.f, float z = 1.f);

    // change the color of the text
    virtual void setColor(glm::fvec3 color);

    // pure virtual function to render the text to screen
    virtual void render(GLuint shaderProgram, float scale) const = 0;


  protected:

    std::shared_ptr<Font> font_;
    std::string text_;
    glm::ivec3 color_;
    glm::fvec3 pos_;

    // width and height of the application window
    float winWidth_, winHeight_;

    // Vertex Array Object and Vertex Buffer Object handlers
    GLuint VAO, VBO;

    // prepare VAO and VBO for rendering
    virtual void prepare() = 0;
};

#endif
