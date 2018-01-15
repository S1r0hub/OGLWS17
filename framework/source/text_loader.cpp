// APPLICATION SOLAR - Text Loader Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#include "text_loader.hpp"


TextLoader::~TextLoader()
{
  //cleanupResources(); // already done after loading
}


void TextLoader::cleanupResources()
{
  for (auto const& font : fonts)
  { if (font.face != nullptr) { FT_Done_Face(font.face); } }

  if (ftlib == nullptr)
  { std::cout << "WARN: FreeType Library was a nullptr!" << std::endl; }
  else { FT_Done_FreeType(ftlib); }
}


bool TextLoader::initializeFreeTypeLibrary()
{
  // initialize the freetype library
  if (FT_Init_FreeType(&ftlib)) { return false; }
  return true;
}


void TextLoader::addFont(std::string name, std::string fontPath, int height, int width)
{
  Font newFont = Font{name, fontPath, height, width};
  fonts.push_back(newFont);
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
    font.face = face;

    // font settings
    FT_Set_Pixel_Sizes(face, font.width, font.height); // set font width and height (0 = dynamically calculate)
 
    // pre-load the font characters
    loadFontCharacters(font);

    std::cout << "Font \"" << font.name << "\" initialized." << std::endl;
    font.loaded = true;
  }

  // clean up resources used by FreeType
  cleanupResources();

  return true;
}


void TextLoader::loadFontCharacters(Font& font)
{
  // disable byte-alignment restriction
  // (because we will only use a single byte (GL_RED) to represent the colors of the textures)
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // load the first 128 characters of the ASCII character set
  for (GLubyte c = 0; c < 128; c++)
  {
    // load the character glyph and set it as the active one for the face
    if (!setActiveFaceCharacter(font.face, c)) { continue; } // skip if failed

    // generate the texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RED, // because the bitmap is just 8-bit grayscale
      font.face->glyph->bitmap.width,
      font.face->glyph->bitmap.rows,
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      font.face->glyph->bitmap.buffer
    );

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // store the generated character information
    TextCharacter tc = TextCharacter{
      texture,
      font.face->glyph->bitmap.width,
      font.face->glyph->bitmap.rows,
      font.face->glyph->bitmap_left,
      font.face->glyph->bitmap_top,
      (GLuint) font.face->glyph->advance.x
    };

    // add the character information to the map
    font.characters[c] = tc;
  }
}


bool TextLoader::setActiveFaceCharacter(FT_Face face, char character)
{
  // FT_LOAD_RENDER creates an 8-bit grayscale bitmap image (face->glyph->bitmap)
  if (FT_Load_Char(face, character, FT_LOAD_RENDER))
  {
    std::cerr << "ERROR: FreeType failed to load glyph for character (" << character << ")!" << std::endl;
    return false;
  }
  return true;
}
