#include "application_solar.hpp"
#include "launcher.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// to print out a matrix or vector (debug related)
#include <glm/gtx/string_cast.hpp>

#include <iostream>


ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
{
  initializeGeometry();
  initializeShaderPrograms();
  initializePlanets();
}


void ApplicationSolar::renderPlanet(std::shared_ptr<Planet> planet) const
{
  // set the origin of the planet (where it will rotate around)
  glm::fmat4 model_matrix = glm::translate(glm::fmat4{}, planet->getOrigin());

  // rotate planet around its origin (https://glm.g-truc.net/0.9.2/api/a00245.html)
  // (rotate the matrix by an angle (here by time) using an axis vector)
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getOrbitRotationAngle(time_multiplier), planet->getRotationDir());

  // translate the planet position after rotating it (changing its orbit)
  model_matrix = glm::translate(model_matrix, planet->getOrbitTranslation());

  // rotate the planet itself
  // correct but bad because of the sun model:
  //model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getRotationAngle() * time_multiplier, planet->getRotationDir());
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getRotationAngle(time_multiplier), planet->getRotationDir());

  // scale the planet
  model_matrix = glm::scale(model_matrix, glm::fvec3(1.0f, 1.0f, 1.0f) * planet->getSize());


  // upload model matrix to shader
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

  // extra matrix for normal transformation to keep them orthogonal to surface
  glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));


  // bind the VAO to draw
  glBindVertexArray(planet_object.vertex_AO);

  // draw bound vertex array using bound shader
  glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
}


void ApplicationSolar::render() const
{
  // bind shader to upload the following uniforms
  glUseProgram(m_shaders.at("planet").handle);

  for (int i = 0; i < planets.size(); i++)
  { renderPlanet(planets.at(i)); }
}


void ApplicationSolar::updateView()
{
  // rotate the camera now also around the x-axis using the rotationVectorX
  m_view_transform = glm::rotate(m_view_transform, cameraRotationX, glm::fvec3{1.0f, 0.0f, 0.0f});

  // move the camera to the position it is aiming at depending on the movementVector
  m_view_transform = glm::translate(m_view_transform, movementVector);

  // reset movementVector now so that we do not always move when a view update occurs
  movementVector = glm::fvec3{};

  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
 
  // undo the matrix x-axis rotation to avoid weird camera behavoir next rotation (this will lock z-axis)
  m_view_transform = glm::rotate(m_view_transform, -cameraRotationX, glm::fvec3{1.0f, 0.0f, 0.0f});

  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
}


void ApplicationSolar::updateProjection()
{
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));
}


// update uniform locations
void ApplicationSolar::uploadUniforms()
{
  updateUniformLocations();
  
  // bind new shader
  glUseProgram(m_shaders.at("planet").handle);

  updateView();
  updateProjection();
}


// handle key input
void ApplicationSolar::keyCallback(int key, int scancode, int action, int mods)
{
  // movement
  // actions: GLFW_PRESS, GLFW_REPEAT, GLFW_RELEASE
  if (action == GLFW_PRESS || action == GLFW_RELEASE)
  {
    bool active = false;

    if (action == GLFW_PRESS) { active = true; }
    else if (action == GLFW_RELEASE) { active = false; }

    switch (key)
    {
      // keys for movement
      case GLFW_KEY_W: moveForward = active; break;
      case GLFW_KEY_S: moveBackward = active; break;
      case GLFW_KEY_A: moveLeft = active; break;
      case GLFW_KEY_D: moveRight = active; break;
      case GLFW_KEY_SPACE: moveUp = active; break;
      case GLFW_KEY_C: moveDown = active; break;
      case GLFW_KEY_LEFT_SHIFT: moveFast = active; break;
    }
  }

  move();
}


// do the actual movement by using the coordinates and updating the view
void ApplicationSolar::move()
{
  // ensure that we even move in any direction
  bool moving = moveForward || moveBackward || moveLeft || moveRight || moveUp || moveDown;

  if (moving)
  {
    movementVector = glm::fvec3{};

    // calculate the camera movement speed
    float speed = cameraSpeed * (moveFast ? cameraSprintMultiplier : 1.f);

    if (moveForward)
    { movementVector += glm::fvec3{0.0f, 0.0f, -1.0f} * speed; }
    else if (moveBackward)
    { movementVector += glm::fvec3{0.0f, 0.0f, 1.0f} * speed; }
  
    if (moveLeft)
    { movementVector += glm::fvec3{-1.0f, 0.0f, 0.0f} * speed; }
    else if (moveRight)
    { movementVector += glm::fvec3{1.0f, 0.0f, 0.0f} * speed; }

    if (moveUp)
    { movementVector += glm::fvec3{0.0f, 1.0f, 0.0f} * speed; }
    else if (moveDown)
    { movementVector += glm::fvec3{0.0f, -1.0f, 0.0f} * speed; }

    // If we move on x and z at the same time
    // the speed would be double as fast, so we have to divide in this case:
    if (movementVector.x != 0 && movementVector.z != 0)
    { movementVector *= glm::fvec3{0.5f, 0.5f, 1.0f}; }

    updateView();
  }
}



//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y)
{
  // left = pos_x = -1, right = pos_x = 1
  // up = pos_y = -1, down = pos_y = 1


  // Ensure the values are always -1 , 0 or 1
  // because there was a weird high value after the first movement.
  pos_x = pos_x > 1 ? 1 : pos_x < -1 ? -1 : pos_x;
  pos_y = pos_y > 1 ? 1 : pos_y < -1 ? -1 : pos_y;

  // rotate the view around the y-axis (left and right)
  m_view_transform = glm::rotate(m_view_transform, (float) (cameraRotationSpeed * -pos_x), glm::fvec3{0.0f, 1.0f, 0.0f});

  // store the rotation up and down and apply it in updateView()
  float camRotXAdd = (float) (cameraRotationSpeed * -pos_y);

  // ensure we only rotate 90 degree up and -90 degree down
  // to avoid inverted camera rotation
  if (cameraRotationX + camRotXAdd > cameraRotationX_max)
  { cameraRotationX = cameraRotationX_max; }
  else if (cameraRotationX + camRotXAdd < cameraRotationX_min)
  { cameraRotationX = cameraRotationX_min; }
  else { cameraRotationX += camRotXAdd; }

  updateView();
}


// load shader programs
void ApplicationSolar::initializeShaderPrograms()
{
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/simple.vert",
                                           m_resource_path + "shaders/simple.frag"});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
}


// load models
void ApplicationSolar::initializeGeometry()
{
  //model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);
  model planet_model = model_loader::obj(m_resource_path + "models/sphere_own.obj", model::NORMAL);
  //model planet_model = model_loader::obj(m_resource_path + "models/test_cube.obj", model::NORMAL);
  //model planet_model = model_loader::obj(m_resource_path + "models/test_monkey.obj", model::NORMAL);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO); // = Array Object
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);
  
  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);

  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);


  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);


  // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);

  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);


  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;

  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}


// initialize all the different planet objects
void ApplicationSolar::initializePlanets()
{
  if (!planets.empty()) { planets.clear(); }


  // sizes according to this data: https://www.timeanddate.de/astronomie/planeten/groesse-reihenfolge

  float system_scale = 100000;

  // original data in km divided by system_scale = real representation of our solar system
  // DONT DELETE!
  float size_sun = 1391000.f / system_scale,
        size_mercury = 4879.f / system_scale,
        size_venus = 12104.f / system_scale,
        size_earth = 12756.f / system_scale,
        size_mars = 6792.f / system_scale,
        size_jupiter = 142984.f / system_scale,
        size_saturn = 120536.f / system_scale,
        size_uranus = 51118.f / system_scale,
        size_neptune = 49528.f / system_scale;

  // fictional values for a nice and close to reality representation
  /*
  float size_sun = 4,
        size_mercury = 0.2f,
        size_venus = 0.5f,
        size_earth = 0.5f,
        size_mars = 0.3f,
        size_jupiter = 1,
        size_saturn = 1,
        size_uranus = 0.7f,
        size_neptune = 0.7f;
  */

  // distance from sun (first number is in million km thats why * 100000 at the end)
  // following real distances dont work because the whole system would be too large to render then
  // DONT DELETE!
  /*
  float dist_sun = 0 / system_scale,
        dist_mercury = 68129 / system_scale * 100000,
        dist_venus = 107559 / system_scale * 100000,
        dist_earth = 148954 / system_scale * 100000,
        dist_mars = 249126 / system_scale * 100000,
        dist_jupiter = 814108 / system_scale * 100000,
        dist_saturn = 1505417 / system_scale * 100000,
        dist_uranus = 2978554 / system_scale * 100000,
        dist_neptune = 4479866 / system_scale * 100000;
  */

  float dist_sun = 0,
        dist_mercury = dist_sun + size_sun + size_mercury + 1,
        dist_venus = dist_mercury + size_mercury + size_venus + 1,
        dist_earth = dist_venus + size_venus + size_earth + 1,
        dist_mars = dist_earth + size_earth + size_mars + 1,
        dist_jupiter = dist_mars + size_mars + size_jupiter + 1,
        dist_saturn = dist_jupiter + size_jupiter + size_saturn + 1,
        dist_uranus = dist_saturn + size_saturn + size_uranus + 1,
        dist_neptune = dist_uranus + size_uranus + size_neptune + 1;


  // planet orbit speed https://de.wikipedia.org/wiki/Umlaufzeit
  // and https://astrokramkiste.de/planeten-tabelle (also for day time)

  // real orbit time of the planets (in days)
  float ot_sun = 0, // is almost zero because of around 230 Mil. years
        ot_mercury = 87.969f,
        ot_venus = 224.701f,
        ot_earth = 365.256f,
        ot_mars = 686.980f,
        ot_jupiter = 4329.6f,   // 11.862 years
        ot_saturn = 10752.2f,   // 29.458 years
        ot_uranus = 30665.1f,   // 84.014 years
        ot_neptune = 60149.45f; // 164.793 years

  // real day time of the planets (in days = 24 hours)
  float dt_sun = 28, // between 25 days and 9 hours and 31 days and 19 hours
        dt_mercury = 58.625f,  // 58 days and 15 hours
        dt_venus = 243.f,      // 243 days
        dt_earth = 1.f,
        dt_mars = 1.02f,       // 1 day, 30 min
        dt_jupiter = 0.42f,    // approx 10 h
        dt_saturn = 0.45f,     // 10 h, 45 m
        dt_uranus = 0.72f,     // 17 h, 15 m
        dt_neptune = 0.67f;    // approx 16 h


  // create and add all the planets to our list of planets
  std::cout << "Sun has a size of " << size_sun << std::endl;
  planets.push_back(std::make_shared<Planet>("Sun", size_sun, ot_sun, dt_sun));
  planets.push_back(std::make_shared<Planet>("Mercury", size_mercury, ot_mercury, dt_mercury, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_mercury}));
  planets.push_back(std::make_shared<Planet>("Venus", size_venus, ot_venus, dt_venus, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_venus}));
  planets.push_back(std::make_shared<Planet>("Earth", size_earth, ot_earth, dt_earth, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_earth}));
  // TODO: moon
  planets.push_back(std::make_shared<Planet>("Mars", size_mars, ot_mars, dt_mars, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_mars}));
  planets.push_back(std::make_shared<Planet>("Jupiter", size_jupiter, ot_jupiter, dt_jupiter, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_jupiter}));
  planets.push_back(std::make_shared<Planet>("Saturn", size_saturn, ot_saturn, dt_saturn, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_saturn}));
  planets.push_back(std::make_shared<Planet>("Uranus", size_uranus, ot_uranus, dt_uranus, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_uranus}));
  planets.push_back(std::make_shared<Planet>("Neptun", size_uranus, ot_neptune, dt_neptune, glm::fvec3{}, glm::fvec3{0.0f, 0.0f, dist_neptune}));

  std::cout << "Rotation angle for \"" << planets.at(0)->getName() << "\" per second is: " << planets.at(0)->getRotationAngle(time_multiplier) << std::endl;
}


ApplicationSolar::~ApplicationSolar()
{
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}


// exe entry point
int main(int argc, char* argv[])
{
  Launcher::run<ApplicationSolar>(argc, argv);
}