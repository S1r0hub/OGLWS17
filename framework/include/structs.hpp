#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include <map>
#include <glbinding/gl/gl.h>

#include <string> // for planet names
#include <glm/glm.hpp> // for planet positions

// use gl definitions from glbinding 
using namespace gl;


// gpu representation of model
struct model_object
{
  // vertex array object
  GLuint vertex_AO = 0;
  // vertex buffer object
  GLuint vertex_BO = 0;
  // index buffer object
  GLuint element_BO = 0;
  // primitive type to draw
  GLenum draw_mode = GL_NONE;
  // indices number, if EBO exists
  GLsizei num_elements = 0;
};


struct planet
{
  planet(std::string name_) : name(name_) {}
  planet(std::string name_, glm::fvec3 origin_, glm::fvec3 rotationDir_, glm::fvec3 translation_)
    : name(name_),
      origin(origin_),
      rotationDir(rotationDir_),
      translation(translation_)
  {}

  std::string name = "Unknown";

  glm::fvec3 origin{0.0f, 0.0f, 0.0f};

  // self rotation direction
  glm::fvec3 rotationDir{0.0f, 1.0f, 0.0f};

  // ...
  glm::fvec3 translation{0.0f, 0.0f, 0.0f};

  float size = 1.0f,
        rotationSpeed = 1.0f;
};


// gpu representation of texture
struct texture_object
{
  // handle of texture object
  GLuint handle = 0;
  // binding point
  GLenum target = GL_NONE;
};


// shader handle and uniform storage
struct shader_program
{
  shader_program(std::string const& vertex, std::string const& fragment)
    :vertex_path{vertex}
    ,fragment_path{fragment}
    ,handle{0}
    {}

  // path to shader source
  std::string vertex_path; 
  std::string fragment_path; 
  // object handle
  GLuint handle;
  // uniform locations mapped to name
  std::map<std::string, GLint> u_locs{};
};

#endif