#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

// added for assignment 1
#include <deque> // holding pointers to the planet objects
#include <memory> // included to use shared pointers (std::shared_ptr)
#include "Planet.hpp"


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

    // upload planet information
    void renderPlanet(std::shared_ptr<Planet> planet) const;


  protected:

    void initializeShaderPrograms();
    void initializeGeometry();
    void initializePlanets();
    void updateView();
    void move();

    // cpu representation of model
    model_object planet_object;

    // simulation speed used by planets (1 = real time)
    float time_multiplier = 100000.f;

    // deque holding pointers for the planet objects
    std::deque<std::shared_ptr<Planet>> planets;

    // holding movement information
    bool moveForward = false,
	       moveBackward = false,
	       moveLeft = false,
	       moveRight = false,
         moveUp = false,
         moveDown = false;
};

#endif