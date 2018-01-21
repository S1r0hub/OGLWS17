// APPLICATION SOLAR - Text Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#include "text.hpp"


Text::Text(std::string text, Font& font, glm::fvec3 pos, glm::fvec3 color, unsigned int winWidth, unsigned int winHeight)
  : text_(text)
  , font_(font)
  , pos_(pos)
  , color_(color)
  , winWidth_(winWidth)
  , winHeight_(winHeight)
{}



// #### GETTER #### //

glm::fvec3 Text::getPosition()
{
  return pos_;
}

glm::fvec3 Text::getColor()
{
  return color_;
}

std::string Text::getText()
{
  return text_;
}



// #### SETTER #### //

void Text::setText(std::string text)
{
  text_ = text;
}

void Text::setPosition(glm::fvec3 pos)
{
  pos_ = pos;
}

void Text::setPosition(float x, float y, float z)
{
  pos_ = glm::fvec3{x,y,z};
}

void Text::setColor(glm::fvec3 color)
{
  color_ = color;
}


