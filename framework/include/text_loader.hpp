// APPLICATION SOLAR - Text Loader Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef SOLARSYS_TEXT_LOADER
#define SOLARSYS_TEXT_LOADER


#include <iostream>
#include <vector>

// structs and several included libraries
#include "text_structs.hpp"


/*
  Usage:
  1. Add Fonts using the "addFont" method
  2. Load everything using the "load" method
*/
class TextLoader
{
  public:

    // clean up resources used by freetype
    ~TextLoader();

    // Loads and stores a font and returns if this operation was successful.
    // Leave width or height at 0 to calculate it dynamically
    void addFont(std::string name, std::string fontPath, int height = 0, int width = 0);

    // Load all fonts and the glyphs
    bool load();


  private:

    // initialize the used library
    bool initializeFreeTypeLibrary();

    // pre-load all characters for every font
    bool loadFontCharacters(Font& font);

    // load and set the active glyph (data is then accessible using the face)
    // returns true if successful, false otherwise
    bool setActiveFaceCharacter(FT_Face face, char character);

    // cleans the used resources of FreeType
    void cleanupResources();

    FT_Library ftlib;
    std::vector<Font> fonts;
};

#endif
