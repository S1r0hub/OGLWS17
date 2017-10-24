// APPLICATION SOLAR - Planet Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#include "Planet.hpp"


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
float Planet::getPlanetDayTime() { return planetDayTime; }
float Planet::getSize() { return size; }

std::deque<std::shared_ptr<Planet>>& Planet::getMoons()
{ return moons; }

int Planet::getMoonCount()
{ return moons.size(); }

glm::fmat4 Planet::getModelMatrix()
{ return model_matrix; }



// SETTER

void Planet::addMoon(std::shared_ptr<Planet> moon)
{ moons.push_back(moon); }

void Planet::setModelMatrix(glm::fmat4& modelMatrix)
{ model_matrix = modelMatrix; }



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
