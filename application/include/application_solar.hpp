#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

#include <deque>
#include <memory> // include to use shared_ptr


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
  void renderPlanet(std::shared_ptr<planet> planet) const;

 protected:
  void initializeShaderPrograms();
  void initializeGeometry();
  void initializePlanets();
  void updateView();
  void move();

  // cpu representation of model
  model_object planet_object;

  // deque holding pointers for the planet objects
  std::deque<std::shared_ptr<planet>> planets;

  bool moveForward, moveBackward, moveLeft, moveRight;
};

#endif