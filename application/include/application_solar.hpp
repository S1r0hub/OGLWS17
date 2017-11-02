// APPLICATION SOLAR - Header
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include <iostream>
#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// added for assignment 1
#include <deque> // holding pointers to the planet objects
#include <memory> // included to use shared pointers (std::shared_ptr)
#include "Planet.hpp"

// added for assignment 2
#include <cstdlib> // for random
#include <time.h> // for random seed
#include <vector>


// gpu representation of model
class ApplicationSolar : public Application
{
  public:

    // allocate and initialize objects
    ApplicationSolar(std::string const& resource_path);
    // free allocated objects
    ~ApplicationSolar();

    // update uniform locations and values
    void uploadUniforms();

    // update projection matrix
    void updateProjection();

    // react to key input
    void keyCallback(int key, int scancode, int action, int mods);

    //handle delta mouse movement input
    void mouseCallback(double pos_x, double pos_y);

    // draw all objects
    void render() const;

    // render planets and upload information to gpu
    void renderPlanet(std::shared_ptr<Planet> planet) const;

    // render the orbits of planets and moons
    void renderOrbits(std::shared_ptr<Planet> planet) const;

    // render stars and upload information to gpu
    void renderStars() const;


  protected:

    void initializeShaderPrograms();
    void initializeGeometry();
    void initializePlanets();
    void initializeStars();
    void updateView();
    void move();

    // cpu representation of model
    model_object planet_object;
    model_object stars{};


  private:

    // simulation speed used by planets (1 = real time) (use around 1 million)
    double time_multiplier = 1000000;

    // deque holding pointers for the planet objects
    std::deque<std::shared_ptr<Planet>> planets;

    // definition of pi needed for rotation
    float MATH_HALF_PI = glm::half_pi<float>();

    // holding movement information
    bool moveForward = false,
	       moveBackward = false,
	       moveLeft = false,
	       moveRight = false,
         moveUp = false,
         moveDown = false,
         moveFast = false;

    // how fast the camera is moving and rotating in general
    float cameraSpeed = 0.1f;
    float cameraRotationSpeed = 0.01f;

    // store information about the camera rotation around the x-axis
    float cameraRotationX = 0.f;
    float cameraRotationX_max = MATH_HALF_PI;
    float cameraRotationX_min = -MATH_HALF_PI;

    // how many times faster the camera moves on sprinting
    float cameraSprintMultiplier = 2.0f;

    // store information about how the movement should be done
    glm::fvec3 movementVector{0.f, 2.f, 30.f}; // initial camera position

    unsigned int starCount = 10000;
    float starSize = 1.f;
    int starDistance_min = 30;
    int starDistance_max = 80;
};

#endif