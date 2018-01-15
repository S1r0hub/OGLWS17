// APPLICATION SOLAR - Text Loader Structs
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)
// This HPP file stores structs used for text rendering.


#ifndef SOLARSYS_TEXT_STRUCTS
#define SOLARSYS_TEXT_STRUCTS


#include <string>
#include <map>

// OPENGL includes
#include <glbinding/gl/gl.h>
#include <glm/glm.hpp> // (opengl maths)
using namespace gl;

// include freetype font libraries
#include "ft2build.h"
#include FT_FREETYPE_H


// ===============================================//
// Struct for glyphs
struct TextCharacter
{
  TextCharacter()
    : textureID(0)
    , dimension(glm::ivec2(0, 0))
    , bearing(glm::ivec2(0, 0))
    , advance(0)
  {}

  TextCharacter(GLuint texHandle, unsigned int width, unsigned int height, int offset_left, int offset_top, GLuint advance_)
    : textureID(texHandle)
    , dimension(glm::ivec2(width, height))
    , bearing(offset_left, offset_top)
    , advance(advance_)
  {}

  GLuint textureID;       // handle of texture
  glm::ivec2 dimension;   // size of a glyph
  glm::ivec2 bearing;     // offset from baseline to left/top
  GLuint advance;         // horizontal offset to next glyph
};


// ===============================================//
// Simple font struct for the text loader
struct Font
{
  Font(std::string name_, std::string path_, int height_ = 0, int width_ = 0)
    : name(name_)
    , path(path_)
    , height(height_)
    , width(width_)
  {}

  std::string name = "";
  std::string path = "";
  int width = 0;
  int height = 0;
  bool loaded = false;
  FT_Face face;

  // all loaded characters/gylphs are stored by this map
  std::map<GLchar, TextCharacter> characters;
};


#endif // !SOLARSYS_TEXT_STRUCTS
