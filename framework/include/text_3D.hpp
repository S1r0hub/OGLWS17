// APPLICATION SOLAR - Text3D Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)



#ifndef SOLARSYS_TEXT_3DTEXT
#define SOLARSYS_TEXT_3DTEXT

#include "text.hpp"


// A 3D Text object for on-screen (GUI) text rendering.
class Text3D : public Text
{
  public:

    // the text and integer vector for the color (range 0-1)
    Text3D(std::string text, std::shared_ptr<Font> font, glm::fvec3 pos, glm::fvec3 color = glm::fvec3{1.0f}, unsigned int winWidth = 800, unsigned int winHeight = 600);

    // change projection matrix
    void setModelMatrix(glm::mat4& modelMatrix);

    // change the location of the text
    void setPosition(float x = 1.f, float y = 1.f, float z = 1.f);

    // render to screen
    void render(GLuint shaderProgram, float scale = 1.f) const override;


  private:

    glm::mat4 modelMatrix_;

    // to re-upload only after changes
    mutable bool modelMatrixUpdated = true;

    // prepare VAO and VBO for rendering
    void prepare() override;
};

#endif
