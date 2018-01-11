// APPLICATION SOLAR - Text Loader Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef SOLARSYS_TEXTLOADER
#define SOLARSYS_TEXTLOADER


#include <iostream>
#include <string>
#include <vector>
#include <map>

// include freetype font libraries
#include "ft2build.h"
#include FT_FREETYPE_H


// Font struct for text loader
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
};


/*
  Usage:
  1. Add Fonts
  2. Load everything
*/
class TextLoader
{
  public:

    // clean up resources used by freetype
    ~TextLoader();

    // Loads and stores a font and returns if this operation was successful.
    // Leave width or height at 0 to calculate it dynamically
    void addFont(std::string name, std::string fontPath, int height = 0, int width = 0);

    // Load all fonts
    bool load();


  private:

    // initialize the used library
    bool initializeFreeTypeLibrary();

    void setActiveFaceCharacter(FT_Face face, char character);

    FT_Library ftlib;
    std::vector<Font> fonts;
};

#endif
