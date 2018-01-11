// APPLICATION SOLAR - Text Loader Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#include "text_loader.hpp"


TextLoader::~TextLoader()
{
  for (auto const& font : fonts)
  {
    //std::cout << "Removing font: " << font.name << std::endl;
    if (font.face != nullptr) { FT_Done_Face(font.face); }
  }

  if (ftlib == nullptr)
  { std::cout << "FreeType Library was nullptr!" << std::endl; }
  else { FT_Done_FreeType(ftlib); }
}


bool TextLoader::initializeFreeTypeLibrary()
{
  // initialize the freetype library
  if (FT_Init_FreeType(&ftlib)) { return false; }
  return true;
}


bool TextLoader::load()
{
  if (!initializeFreeTypeLibrary())
  {
    std::cerr << "ERROR: Failed to initialize the FreeType Library!" << std::endl;
    return false;
  }
  std::cout << "FreeType Library initialized." << std::endl;

  for (auto& font : fonts)
  {
    // load the font as a so called "face"
    FT_Face face;
    if (FT_New_Face(ftlib, font.path.c_str(), 0, &face))
    {
      std::cerr << "ERROR: Failed to load font \"" 
        << font.name << "\" from: " 
        << font.path << std::endl;
      continue;
    }

    // font settings
    FT_Set_Pixel_Sizes(face, font.width, font.height); // set font width and height (0 = dynamically calculate)
 
    std::cout << "Font \"" << font.name << "\" initialized." << std::endl;
    font.loaded = true;
  }

  return true;
}


void TextLoader::addFont(std::string name, std::string fontPath, int height, int width)
{
  Font newFont = Font{name, fontPath, height, width};
  fonts.push_back(newFont);
}


void TextLoader::setActiveFaceCharacter(FT_Face face, char character)
{
  // load and set the active glyph (data is then accessible using the face)
  // FT_LOAD_RENDER creates an 8-bit grayscale bitmap image (face->glyph->bitmap)
  if (FT_Load_Char(face, character, FT_LOAD_RENDER))
  { std::cerr << "ERROR: FreeType failed to load a glyph (" << character << ")!" << std::endl; }
}
