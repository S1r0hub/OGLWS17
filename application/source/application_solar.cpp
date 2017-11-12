// APPLICATION SOLAR - Source
// COMPUTER GRAPHICS WiSe 17/18 - Assignments
// Leon H. (115853)
// Marcel H. (116610)


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

// to convert matrix to string.. (debug)
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_access.hpp>


ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
{
  initializeGeometry();
  initializeShaderPrograms();
  initializeStars();
  initializePlanets();
}


void ApplicationSolar::renderObject(std::shared_ptr<Planet> planet) const
{
  // first render orbits before translating position of planets
  renderOrbits(planet);

  // set the origin of the planet (where it will rotate around)
  glm::fmat4 model_matrix = planet->getModelMatrix();

  // rotate planet around its origin (https://glm.g-truc.net/0.9.2/api/a00245.html)
  // (rotate the matrix by an angle (here by time) using an axis vector)
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getOrbitRotationAngle(time_multiplier), planet->getRotationDir());

  // translate the planet position after rotating it (changing its orbit)
  model_matrix = glm::translate(model_matrix, planet->getOrbitTranslation());

  // render the moons of the planet or the planets of a sun
  // We just assume that the moons of a sun are the planets :D
  std::deque<std::shared_ptr<Planet>> moons = planet->getMoons();
  std::shared_ptr<Planet> moon;
  for (unsigned int i = 0; i < moons.size(); i++)
  {
    moon = moons.at(i);
    // use the current model matrix of the origin planet (transformed to position)
    moon->setModelMatrix(model_matrix);
    renderObject(moon);
  }

  // rotate the planet itself
  // correct but bad because of the sun model:
  //model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getRotationAngle() * time_multiplier, planet->getRotationDir());
  model_matrix = glm::rotate(model_matrix, float(glfwGetTime()) * planet->getRotationAngle(time_multiplier), planet->getRotationDir());

  // scale the planet
  model_matrix = glm::scale(model_matrix, glm::fvec3(1.0f, 1.0f, 1.0f) * planet->getSize());


  // use planet color for shaders
  float* planetColor = planet->getColor();

  // render the sun
  if (planet->isSun()) // use sun shader
  {
    // use sun shader program
    glUseProgram(m_shaders.at("sun").handle);

    // we would prefer to pass a vector here but the assignment tells us to use Uniform3f
    glUniform3f(m_shaders.at("sun").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);

    // upload model matrix to shader
    glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    //glEnable(GL_DEPTH_TEST); // already enabled by launcher.cpp render function
    glEnable(GL_CULL_FACE);  // enable culling so we don't render back faces
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
  else
  {
    // render planets and moons
    if (shadingMode == 0) // blinn phong shading
    {
      glUseProgram(m_shaders.at("planet").handle);

      // we would prefer to pass a vector here but the assignment tells us to use Uniform3f
      glUniform3f(m_shaders.at("planet").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);

      // upload model matrix to shader
      glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

      // extra matrix for normal transformation to keep them orthogonal to surface
      //glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
      glm::fmat4 normal_matrix = glm::inverseTranspose(view_matrix * model_matrix);
      glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));

      // bind the VAO to draw
      glBindVertexArray(planet_object.vertex_AO);

      // draw bound vertex array using bound shader
      //glEnable(GL_DEPTH_TEST); // already enabled by launcher.cpp render function
      glEnable(GL_CULL_FACE);  // enable culling so we don't render back faces
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
    }
    else if (shadingMode == 1) // cel shading
    {
      // bind the VAO to draw
      glBindVertexArray(planet_object.vertex_AO);

      // 1. draw the silhouette
      glUseProgram(m_shaders.at("celSilhouette").handle);
      glUniform3fv(m_shaders.at("celSilhouette").u_locs.at("BorderColor"), 1, borderColor.data());
      glUniformMatrix4fv(m_shaders.at("celSilhouette").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));
  
      // draw the object just 2d with the border color
      glEnable(GL_CULL_FACE); // enable culling
      glCullFace(GL_FRONT);   // enable culling of front faces (render back faces)
      //glDepthMask(GL_TRUE); // enable writes to Z-buffer (needed for texture cel shading later)
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

      // 2. draw the cel shading color
      glUseProgram(m_shaders.at("celColor").handle);
      glUniform3f(m_shaders.at("celColor").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);
      glUniformMatrix4fv(m_shaders.at("celColor").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

      // draw the 3d object layer and add it to the 2d border layer
      glCullFace(GL_BACK);     // enable culling of back faces (don't render them)
      //glDepthMask(GL_FALSE); // disable writes to Z-buffer (needed for texture cel shading later)
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
    }
  }
}


void ApplicationSolar::renderOrbits(std::shared_ptr<Planet> planet) const
{
  if (planet->getOrbitPointCount() <= 0) { return; }

  // RENDER ORBITS
  glUseProgram(m_shaders.at("orbits").handle);

  glUniform3fv(m_shaders.at("orbits").u_locs.at("OrbitColor"), 1, planet->getOrbitColor());
  glUniformMatrix4fv(m_shaders.at("orbits").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(planet->getModelMatrix()));

  // draw primitive
  glBegin(GL_LINE_LOOP);
  for (glm::fvec2 v : planet->getOrbitPoints())
  { glVertex3f(v.x, 0, v.y); }
  glEnd();
}


void ApplicationSolar::renderStars() const
{
  // use the shader program for the stars
  glUseProgram(m_shaders.at("stars").handle);

  // bind the VAO to draw, set the point size and draw
  glBindVertexArray(stars.vertex_AO);
  glPointSize(starSize);
  glDrawArrays(stars.draw_mode, 0, starCount);
}


void ApplicationSolar::render() const
{
  renderStars();
  renderObject(sun); // render the sun and all their planets with their moons

  // We sadly have to use a mutable here because the
  // framework does not provide a main loop and
  // we don't want to change it too much.
  double currentTimestamp = glfwGetTime();
  deltaTime = float(currentTimestamp - lastTimestamp);
  lastTimestamp = currentTimestamp;
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
  //glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  view_matrix = glm::inverse(m_view_transform);

  // undo the matrix x-axis rotation to avoid weird camera behavoir next rotation (this will lock z-axis)
  m_view_transform = glm::rotate(m_view_transform, -cameraRotationX, glm::fvec3{1.0f, 0.0f, 0.0f});

  // upload view matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
  
  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("stars").handle);
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("orbits").handle);
  glUniformMatrix4fv(m_shaders.at("orbits").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("celSilhouette").handle);
  glUniformMatrix4fv(m_shaders.at("celSilhouette").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));

  glUseProgram(m_shaders.at("celColor").handle);
  glUniformMatrix4fv(m_shaders.at("celColor").u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
}


void ApplicationSolar::updateProjection()
{
  // upload projection matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));
  
  glUseProgram(m_shaders.at("sun").handle);
  glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("stars").handle);
  glUniformMatrix4fv(m_shaders.at("stars").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("orbits").handle);
  glUniformMatrix4fv(m_shaders.at("orbits").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("celSilhouette").handle);
  glUniformMatrix4fv(m_shaders.at("celSilhouette").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));

  glUseProgram(m_shaders.at("celColor").handle);
  glUniformMatrix4fv(m_shaders.at("celColor").u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));
}


// update uniform locations
void ApplicationSolar::uploadUniforms()
{
  updateUniformLocations();
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

      case GLFW_KEY_1:
        if (shadingMode == 0) { break; }
        std::cout << "Using Blinn Phong Shading now." << std::endl;
        shadingMode = 0;
        break;

      case GLFW_KEY_2:
        if (shadingMode == 1) { break; }
        std::cout << "Using Cel Shading now." << std::endl;
        shadingMode = 1;
        break;

      case GLFW_KEY_P:
        // debug - to check camera position and view matrix
        std::cout << "--------------------" << std::endl;
        std::cout << "View Mat:" << std::endl;
        std::cout << glm::to_string(view_matrix) << std::endl;
        std::cout << "Camera Position:" << std::endl;
        glm::fmat4 ivm = glm::inverse(view_matrix);
        glm::fvec4 camPos = glm::column(ivm, 3);
        std::cout << glm::to_string(camPos) << std::endl;
        std::cout << "Camera Position Normalized:" << std::endl;
        std::cout << glm::to_string(glm::normalize(camPos)) << std::endl;
        std::cout << "--------------------" << std::endl;
        break;
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
    { movementVector += glm::fvec3{0.0f, 0.0f, -1.0f} * deltaTime * speed; }
    else if (moveBackward)
    { movementVector += glm::fvec3{0.0f, 0.0f, 1.0f} * deltaTime * speed; }
  
    if (moveLeft)
    { movementVector += glm::fvec3{-1.0f, 0.0f, 0.0f} * deltaTime * speed; }
    else if (moveRight)
    { movementVector += glm::fvec3{1.0f, 0.0f, 0.0f} * deltaTime * speed; }

    if (moveUp)
    { movementVector += glm::fvec3{0.0f, 1.0f, 0.0f} * deltaTime * speed; }
    else if (moveDown)
    { movementVector += glm::fvec3{0.0f, -1.0f, 0.0f} * deltaTime * speed; }

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

  // Ensure the values are always -1 and 1
  // because there was a weird high value after the first movement.
  pos_x = glm::clamp(pos_x, -1.0, 1.0);
  pos_y = glm::clamp(pos_y, -1.0, 1.0);

  // rotate the view around the y-axis (left and right)
  m_view_transform = glm::rotate(m_view_transform, (float) (cameraRotationSpeed * deltaTime * -pos_x), glm::fvec3{0.0f, 1.0f, 0.0f});

  // store the rotation up and down and apply it in updateView()
  float camRotXAdd = (float) (cameraRotationSpeed * deltaTime * -pos_y);

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
  // ===================================================
  // shader programs for planets and suns
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/planet.vert",
                                             m_resource_path + "shaders/planet.frag"});

  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/sun.vert",
                                          m_resource_path + "shaders/sun.frag"});

  // request uniform locations for planet shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["Color"] = -1;

  // request uniform locations for sun shader program
  //m_shaders.at("sun").u_locs["NormalMatrix"] = -1; // not needed currently
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  m_shaders.at("sun").u_locs["ViewMatrix"] = -1;
  m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("sun").u_locs["Color"] = -1;

  // ===================================================


  // ===================================================
  // shader programs for cel shading

  m_shaders.emplace("celSilhouette", shader_program{m_resource_path + "shaders/cel_silhouette.vert",
                                                    m_resource_path + "shaders/cel_silhouette.frag"});

  m_shaders.emplace("celColor", shader_program{m_resource_path + "shaders/cel_color.vert",
                                               m_resource_path + "shaders/cel_color.frag"});

  // request uniform locations for cel silhouette shader program
  m_shaders.at("celSilhouette").u_locs["ModelMatrix"] = -1;
  m_shaders.at("celSilhouette").u_locs["ViewMatrix"] = -1;
  m_shaders.at("celSilhouette").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("celSilhouette").u_locs["BorderColor"] = -1;

  // request uniform locations for cel color shader program
  m_shaders.at("celColor").u_locs["ModelMatrix"] = -1;
  m_shaders.at("celColor").u_locs["ViewMatrix"] = -1;
  m_shaders.at("celColor").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("celColor").u_locs["Color"] = -1;

  // ===================================================


  // shader for stars
  m_shaders.emplace("stars", shader_program{m_resource_path + "shaders/stars.vert",
                                            m_resource_path + "shaders/stars.frag"});
  // uniform for star colors
  m_shaders.at("stars").u_locs["ViewMatrix"] = -1;
  m_shaders.at("stars").u_locs["ProjectionMatrix"] = -1;


  // shader for orbits
  m_shaders.emplace("orbits", shader_program{m_resource_path + "shaders/orbits.vert",
                                             m_resource_path + "shaders/orbits.frag"});
  m_shaders.at("orbits").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbits").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbits").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("orbits").u_locs["OrbitColor"] = -1;
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


void ApplicationSolar::initializeStars()
{
  // vector including position and color of stars
  std::vector<float> starVerts{};

  // generate random star positions

  // set random seed depending on time
  srand(time(NULL));

  float positive = 1;
  float r, g, b;
  float distance;
  glm::fvec3 vec{};

  for (unsigned int i = 0; i < starCount; i++)
  {
    positive = rand() % 2 == 0 ? 1.f : -1.f;
    vec.x = rand() * positive;

    positive = rand() % 2 == 0 ? 1.f : -1.f;
    vec.y = rand() * positive;

    positive = rand() % 2 == 0 ? 1.f : -1.f;
    vec.z = rand() * positive;

    vec = glm::normalize(vec);

    // get random star distance
    distance = rand() % starDistance_max + starDistance_min;

    // apply star distance
    vec *= distance;

    r = rand() % 255 / 255.f;
    g = rand() % 255 / 255.f;
    b = rand() % 255 / 255.f;

    starVerts.push_back(vec.x);
    starVerts.push_back(vec.y);
    starVerts.push_back(vec.z);

    starVerts.push_back(r);
    starVerts.push_back(g);
    starVerts.push_back(b);
  }

  // vertex array object
  glGenVertexArrays(1, &stars.vertex_AO);
  glBindVertexArray(stars.vertex_AO);


  // buffer vertices
  glGenBuffers(1, &stars.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, stars.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, starVerts.size() * sizeof(float), starVerts.data(), GL_STATIC_DRAW);


  // activate first attribute on gpu for position
  glEnableVertexAttribArray(0);

  // first attribute is 3 floats with 3 floats stride and no offset
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);


  // activate second attribute on gpu for the star color
  glEnableVertexAttribArray(1);

  // second attribute is 3 floats with 3 floats stride and 3 floats offset
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));


  // draw points instead of triangles
  stars.draw_mode = GL_POINTS;
  stars.num_elements = starVerts.size() / 6;
}


// initialize all the different planet objects
void ApplicationSolar::initializePlanets()
{
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

  // diameter of moons (in km)
  float size_m_earth = 3476.f / system_scale;


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


  // distance from sun (in km)
  // following real distances dont work because the whole system would be too large to render then
  // DONT DELETE!

  /*
  float system_scale_distance = 1000000;

  float dist_sun = 0 / system_scale_distance,
        dist_mercury = 69312000 / system_scale_distance,
        dist_venus = 107602000 / system_scale_distance,
        dist_earth = 148783000 / system_scale_distance,
        dist_mars = 249037000 / system_scale_distance,
        dist_jupiter = 814035000 / system_scale_distance,
        dist_saturn = 1505417000 / system_scale_distance,
        dist_uranus = 2978554000 / system_scale_distance,
        dist_neptune = 4479866000 / system_scale_distance;
 
  float dist_m_earth = 384400 / system_scale_distance;
  */


  // fictional distance values for the planets and moons
  float dist_sun = 0,
        dist_mercury = dist_sun + size_sun + size_mercury + 1,
        dist_venus = dist_mercury + size_mercury + size_venus + 1,
        dist_earth = dist_venus + size_venus + size_earth + 1,
        dist_mars = dist_earth + size_earth + size_mars + 1,
        dist_jupiter = dist_mars + size_mars + size_jupiter + 1,
        dist_saturn = dist_jupiter + size_jupiter + size_saturn + 1,
        dist_uranus = dist_saturn + size_saturn + size_uranus + 1,
        dist_neptune = dist_uranus + size_uranus + size_neptune + 1;

  float dist_m_earth = size_earth * 2.f;



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

  // real orbit time for moons
  float ot_m_earth = 27.3f;


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

  // real day time of moons (in days)
  float dt_m_earth = 0; // it's actually 27.3 (same as orbit time), 0 does the same in this case

  // test debug
  std::cout << "Sun has a size of " << size_sun << std::endl;


  // create all the planets and set their attributes
  sun = std::make_shared<Planet>("Sun", size_sun, ot_sun, dt_sun);
  sun->setColor(255, 220, 60);
  sun->isSun(true);
  
  std::shared_ptr<Planet> mercury = std::make_shared<Planet>("Mercury", size_mercury, ot_mercury, dt_mercury, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_mercury });
  mercury->setColor(204, 151, 82);
  
  std::shared_ptr<Planet> venus = std::make_shared<Planet>("Venus", size_venus, ot_venus, dt_venus, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_venus });
  venus->setColor(254, 96, 30);

  std::shared_ptr<Planet> earth = std::make_shared<Planet>("Earth", size_earth, ot_earth, dt_earth, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_earth });
  earth->setColor(50, 50, 255);

  std::shared_ptr<Planet> moon1 = std::make_shared<Planet>("Mond", size_m_earth, ot_m_earth, dt_m_earth, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_m_earth });
  moon1->setColor(50, 50, 50);
  earth->addMoon(moon1);
    
  std::shared_ptr<Planet> mars = std::make_shared<Planet>("Mars", size_mars, ot_mars, dt_mars, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_mars });
  mars->setColor(200, 100, 50);
  
  std::shared_ptr<Planet> jupiter = std::make_shared<Planet>("Jupiter", size_jupiter, ot_jupiter, dt_jupiter, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_jupiter });
  jupiter->setColor(200, 110, 38);
  
  std::shared_ptr<Planet> saturn = std::make_shared<Planet>("Saturn", size_saturn, ot_saturn, dt_saturn, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_saturn });
  saturn->setColor(250, 200, 120);
  
  std::shared_ptr<Planet> uranus = std::make_shared<Planet>("Uranus", size_uranus, ot_uranus, dt_uranus, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_uranus });
  uranus->setColor(90, 130, 250);
  
  std::shared_ptr<Planet> neptun = std::make_shared<Planet>("Neptun", size_uranus, ot_neptune, dt_neptune, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_neptune });
  neptun->setColor(100, 120, 200);

  // add colors to list of planets that we want to render
  sun->addMoon(mercury);
  sun->addMoon(venus);
  sun->addMoon(earth);
  sun->addMoon(mars);
  sun->addMoon(jupiter);
  sun->addMoon(saturn);
  sun->addMoon(uranus);
  sun->addMoon(neptun);

  /*
  // old settings
  planets.push_back(mercury);
  planets.push_back(venus);
  planets.push_back(earth);
  planets.push_back(mars);
  planets.push_back(jupiter);
  planets.push_back(saturn);
  planets.push_back(uranus);
  planets.push_back(neptun);
  */


  // test debug
  std::cout << "Rotation angle for \"" << sun->getName() << "\" per second is: " << sun->getRotationAngle(time_multiplier) << std::endl;
}


ApplicationSolar::~ApplicationSolar()
{
  // planets
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  
  // stars
  glDeleteBuffers(1, &stars.vertex_BO);
  //glDeleteBuffers(1, &stars.element_BO);
  glDeleteVertexArrays(1, &stars.vertex_AO);
}


// exe entry point
int main(int argc, char* argv[])
{
  Launcher::run<ApplicationSolar>(argc, argv);
}