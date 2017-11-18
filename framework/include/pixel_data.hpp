#ifndef PIXEL_DATA_HPP
#define PIXEL_DATA_HPP

#include <vector>
#include <cstdint>

// #include <glbinding/gl/types.h>
#include <glbinding/gl/enum.h>
// use gl definitions from glbinding 
using namespace gl;

// holds texture data and format information
struct pixel_data {
  pixel_data()
   :pixels()
   ,width{0}
   ,height{0}
   ,depth{0}
   ,channels{GL_NONE}
   ,channel_type{GL_NONE}
  {}

  pixel_data(std::vector<std::uint8_t> dat, GLenum c, GLenum ty, std::size_t w, std::size_t h = 1, std::size_t d = 1)
   :pixels(dat)
   ,width{w}
   ,height{h}
   ,depth{d}
   ,channels{c}
   ,channel_type{ty}
  {}

  void const* ptr() const {
    return pixels.data();
  }

  std::vector<std::uint8_t> pixels;
  std::size_t width;
  std::size_t height;
  std::size_t depth;

  // pixel format (RGB...)
  GLenum channels;

  // channel format (GL_UNSIGNED_BYTE)
  GLenum channel_type; 
};

#endif