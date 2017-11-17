// APPLICATION SOLAR - Planet Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#include "Planet.hpp"

// include texture loader
#include <stb_image.h>


// CONSTRUCTOR

Planet::Planet(std::string name_, float planetSize, float orbitRotationTime_, float planetDayTime_,
               glm::fvec3 origin_, glm::fvec3 orbitTranslation, glm::fvec3 rotationDir_)
    : name(name_), size(planetSize),
      orbitRotationTime(orbitRotationTime_),
      planetDayTime(planetDayTime_),
      origin(origin_),
      orbitTranslation(orbitTranslation),
      rotationDir(rotationDir_)
{
  // translate to origin
  model_matrix = glm::translate(model_matrix, getOrigin());

  // calculate orbit distance / radius
  orbitRadius = glm::length(orbitTranslation);

  initializeOrbitPoints();
  refreshOrbitRotationAngle();
  refreshSelfRotationAngle();
}



// GETTER

std::string Planet::getName() { return name; }
glm::fvec3 Planet::getOrigin() { return origin; }
glm::fvec3 Planet::getRotationDir() { return rotationDir; }
glm::fvec3 Planet::getOrbitTranslation() { return orbitTranslation; }

// calculate the final rotation angle used by the application
float Planet::getOrbitRotationAngle(double timeMultiplier)
{ return orbitRotationAngle * timeMultiplier / 1000.; }

float Planet::getRotationAngle(double timeMultiplier)
{ return rotationAngle * timeMultiplier / 1000.; }

float Planet::getOrbitRotationTime() { return orbitRotationTime; }

float Planet::getOrbitRadius() { return orbitRadius; }

float* Planet::getOrbitColor() { return orbitColor; }

float* Planet::getColor() { return planetColor; }

float Planet::getOrbitCircumference() { return glm::two_pi<float>() * orbitRadius; }

std::vector<glm::fvec2> Planet::getOrbitPoints() { return orbitPoints; }
unsigned int Planet::getOrbitPointCount() { return orbitPoints.size(); }

float Planet::getPlanetDayTime() { return planetDayTime; }

float Planet::getSize() { return size; }

std::deque<std::shared_ptr<Planet>>& Planet::getMoons()
{ return moons; }

int Planet::getMoonCount()
{ return moons.size(); }

glm::fmat4 Planet::getModelMatrix()
{ return model_matrix; }

bool Planet::isSun() { return sun;  }

// loads a texture and returns true if the process was successful
bool Planet::loadTexture(const char* path, int width, int height, int bit)
{
  //    int x,y,n;
  //    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
  //    // ... process data if not NULL ...
  //    // ... x = width, y = height, n = # 8-bit components per pixel ...
  //    // ... replace '0' with '1'..'4' to force that many components per pixel
  //    // ... but 'n' will always be the number that it would have been if you said 0
  //    stbi_image_free(data)

  imageData = stbi_load(path, &width, &height, &bit, 0);

  // vllt erst im destructor machen -> nochmal nachschauen!!!
  stbi_image_free(imageData);

  if (imageData != nullptr) { return true; }
  return false;
}

// returns pixel data as an unsigned char * or a nullptr if loading failed
unsigned char* Planet::getTextureData()
{ return imageData; }



// SETTER

void Planet::addMoon(std::shared_ptr<Planet> moon)
{ moons.push_back(moon); }

void Planet::setModelMatrix(glm::fmat4& modelMatrix)
{ model_matrix = modelMatrix; }

void Planet::setOrbitColor(int r, int g, int b)
{
  float r_norm = (float) r, g_norm = (float) g, b_norm = (float) b;
  normalizeColor(r_norm, g_norm, b_norm);
  orbitColor[0] = r_norm;
  orbitColor[1] = g_norm;
  orbitColor[2] = b_norm;
}

void Planet::setColor(int r, int g, int b)
{
  float r_norm = (float)r, g_norm = (float)g, b_norm = (float)b;
  normalizeColor(r_norm, g_norm, b_norm);
  planetColor[0] = r_norm;
  planetColor[1] = g_norm;
  planetColor[2] = b_norm;
}

void Planet::isSun(bool isASun)
{ sun = isASun; }



// PRIVATE SETTER

void Planet::refreshOrbitRotationAngle()
{
  // glm uses radians, so two times PI is equal to 360 degree
  float full_rotation = MATH_TWO_PI;

  // 360 degree for one full rotation
  // times 1000 to reduce the digits after comma when we divide it later
  double full_angle = full_rotation * 1000;

  // Ensure to not divide by zero.
  // 24 because of days, 60 because of minutes and again because of seconds
  if (orbitRotationTime == 0.f) { orbitRotationAngle = 0.f; }
  else { orbitRotationAngle = full_angle / (orbitRotationTime * 24.f * 60.f * 60.f); }
}


void Planet::refreshSelfRotationAngle()
{
  float full_rotation = MATH_TWO_PI;

  // like before, just with the self rotation angle
  double full_angle = full_rotation * 1000;

  // Ensure to not divide by zero.
  if (planetDayTime == 0.f) { rotationAngle = 0; }
  else { rotationAngle = full_angle / (planetDayTime * 24 * 60 * 60); }
}


void Planet::initializeOrbitPoints()
{
  // calculate orbit vertices
  float orbitCircumference = getOrbitCircumference();
  if (orbitCircumference <= 0.f) { return; }

  unsigned int orbitPts = std::ceil(orbitCircumference * orbitSegments);
  
  if (orbitPts < orbitSegmentsMin) { orbitPts = orbitSegmentsMin; }

  float a, x, y;

  for (unsigned int i = 0; i < orbitPts; ++i)
  {
    a = glm::two_pi<float>() * (float)i / (float)orbitPts;
    x = getOrbitRadius() * cos(a);
    y = getOrbitRadius() * sin(a);
    orbitPoints.push_back(glm::fvec2{x, y});
  }
}


void Planet::normalizeColor(float& r, float& g, float& b)
{
  r = r > 255 ? 255 : r < 0 ? 0 : r;
  r = r / 255.0f;
  g = g > 255 ? 255 : g < 0 ? 0 : g;
  g = g / 255.0f;
  b = b > 255 ? 255 : b < 0 ? 0 : b;
  b = b / 255.0f;
}