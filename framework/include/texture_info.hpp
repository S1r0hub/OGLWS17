// APPLICATION SOLAR - texture_info source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef SOLAR_SYS_TEXTURE_INFO
#define SOLAR_SYS_TEXTURE_INFO

#include <string>
#include <vector>


#define FLAG_TEX_NORMAL   0x1;
#define FLAG_TEX_SPECULAR 0x2;
#define FLAG_TEX_NIGHT    0x4;


// Struct that holds texture information
struct texture_info
{
  texture_info()
    : index(-1)
    , unit(0)
    , path("")
    , factor(1.0f)
  {}

  texture_info(int index_, int unit_, std::string path_, float factor_ = 1.0f)
    : index(index_)
    , unit(unit_)
    , path(path_)
    , factor(factor_)
  {}

  texture_info(int index_, int unit_, std::vector<std::string> paths_, float factor_ = 1.0f)
    : index(index_)
    , unit(unit_)
    , path("")
    , paths(paths_)
    , factor(factor_)
  {}

  void setFactor(float factor_) { factor = factor_; }

  int index;
  int unit;
  std::string path;
  std::vector<std::string> paths; // needed for cubemaps
  float factor; // for normal mapping and everything that needs a factor
};

#endif
