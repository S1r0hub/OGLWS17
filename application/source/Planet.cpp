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
  refreshOrbitRotationAngle();
  refreshSelfRotationAngle();
}



// GETTER

std::string Planet::getName() { return name; }
glm::fvec3 Planet::getOrigin() { return origin; }
glm::fvec3 Planet::getRotationDir() { return rotationDir; }
glm::fvec3 Planet::getOrbitTranslation() { return orbitTranslation; }
float Planet::getOrbitRotationAngle() { return orbitRotationAngle; }
float Planet::getRotationAngle() { return rotationAngle; }
float Planet::getOrbitRotationTime() { return orbitRotationTime; }
float Planet::getPlanetDayTime() { return planetDayTime; }
float Planet::getSize() { return size; }



// PRIVATE SETTER

void Planet::refreshOrbitRotationAngle()
{
  // definition of one full rotation in glm (not 360 degree!)
  float full_rotation = 3.141592f * 2.0f;

  // 360 degree for one full rotation
  // times 1000 to reduce the digits after comma when we divide it later
  float full_angle = full_rotation * 1000.f;

  // Ensure to not divide by zero.
  // 24 because of days, 60 because of minutes and again because of seconds
  if (orbitRotationTime == 0.f) { orbitRotationAngle = 0.f; }
  else { orbitRotationAngle = full_angle / (orbitRotationTime * 24.f * 60.f * 60.f); }
}


void Planet::refreshSelfRotationAngle()
{
  float full_rotation = 3.141592f * 2.0f;

  // like before, just with the self rotation angle
  float full_angle = full_rotation * 1000.f;

  // Ensure to not divide by zero.
  if (planetDayTime == 0.f) { rotationAngle = 0.f; }
  else { rotationAngle = full_angle / (planetDayTime * 24.f * 60.f * 60.f); }
}
