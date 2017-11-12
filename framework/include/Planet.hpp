// APPLICATION SOLAR - Planet Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef SOLARSYS_PLANET_HPP
#define SOLARSYS_PLANET_HPP

#include <iostream>
#include <string> // included to use std::string for planet names
#include <glm/glm.hpp> // included for glm::vec3f (used by planets)
#include <glm/gtc/constants.hpp> // for constants like pi
#include <glm/gtx/transform.hpp> // for glm::translate, rotate...
#include <deque>
#include <memory>
#include <vector>


// A struct holding information about a planet.
class Planet
{
  // TYPE DEFINITIONS

  typedef glm::fvec3 fv3;


  public:

    // CONSTRUCTOR

    Planet(std::string name_, float planetSize = 1.0f,
           float orbitRotationTime_ = 0.f, float planetDayTime_ = 1.0f,
           glm::fvec3 origin_ = glm::fvec3{0.0f, 0.0f, 0.0f},
           glm::fvec3 orbitTranslation = glm::fvec3{0.0f, 0.0f, 0.0f},
           glm::fvec3 rotationDir_ = glm::fvec3{0.0f, 1.0f, 0.0f});


    // SETTER

    void addMoon(std::shared_ptr<Planet> moon);
    void setModelMatrix(glm::fmat4& modelMatrix);
    void setOrbitColor(int r, int g, int b);

    // in range of 0 to 255
    void setColor(int r, int g, int b);

    void isSun(bool isASun);


    // GETTER

    std::string getName();

    fv3 getOrigin();
    fv3 getRotationDir();
    fv3 getOrbitTranslation();

    float getOrbitRotationAngle(double timeMultiplier);
    float getOrbitRotationTime();
    float getOrbitRadius();
    float* getOrbitColor();
    float* getColor();

    bool isSun();

    std::vector<glm::fvec2> getOrbitPoints();
    unsigned int getOrbitPointCount();

    float getOrbitCircumference();
    float getPlanetDayTime();
    float getRotationAngle(double timeMultiplier);
    float getSize();

    std::deque<std::shared_ptr<Planet>>& getMoons();

    int getMoonCount();

    glm::fmat4 getModelMatrix();


  private:

    // recalculate the orbit rotation angle using the orbit rotation time
    void refreshOrbitRotationAngle();

    // recalculate the angle for the self rotation (rotationAngle)
    void refreshSelfRotationAngle();

    // calculate points for the orbit
    void initializeOrbitPoints();

    // normalize a color from range 0 - 255 to 0 - 1
    void normalizeColor(float& r, float& g, float& b);

    // name of the planet
    std::string name = "Unknown";

    // the origin in space where the planet will rotate around
    fv3 origin{0.0f, 0.0f, 0.0f};

    // rotation direction
    fv3 rotationDir{0.0f, 1.0f, 0.0f};

    // translation of the planet orbit
    fv3 orbitTranslation{0.0f, 0.0f, 0.0f};

    // multiplied with time and affects orbit speed
    double orbitRotationAngle = 1.0f;

    // time (in days) a planet needs to travel on its orbit around its origin
    float orbitRotationTime = 365;

    // time (in days) a planet needs to rotate around itself
    float planetDayTime = 1;

    // affects how fast a planet rotates itself
    double rotationAngle = 0.5f;

    // size of the planet
    float size = 1.0f;

    // definition of pi for rotation
    float MATH_TWO_PI = glm::two_pi<float>();

    // holds all the moons
    std::deque<std::shared_ptr<Planet>> moons{};

	  // model matrix containing position and rotation...
	  glm::fmat4 model_matrix{};

    // distance from origin
    float orbitRadius;
    float orbitColor[3] = { 255, 255, 255 };

    // number of orbit segments per circumference
    float orbitSegments = 1.f;
    unsigned int orbitSegmentsMin = 32;

    // points that form the orbit
    std::vector<glm::fvec2> orbitPoints;

    // planet color
    float planetColor[3] = { 0, 0, 0 };

    // tells if this planet emits light
    bool sun = false;
};

#endif
