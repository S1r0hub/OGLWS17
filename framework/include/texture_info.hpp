// APPLICATION SOLAR - texture_info source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef SOLAR_SYS_TEXTURE_INFO
#define SOLAR_SYS_TEXTURE_INFO

#include <string>

// Struct that holds texture information
struct texture_info
{
  texture_info()
    : index(-1)
    , unit(0)
    , path("")
  {}

  texture_info(int index_, int unit_, std::string path_)
    : index(index_)
    , unit(unit_)
    , path(path_)
  {}

  int index;
  int unit;
  std::string path;
};

#endif
