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


// include texture loader
#include "texture_loader.hpp"


ApplicationSolar::ApplicationSolar(std::string const& resource_path, const unsigned windowWidth, const unsigned windowHeight)
 :Application{resource_path, windowWidth, windowHeight}
{
  std::cout << "Window Dimension: " << windowWidth << "x" << windowHeight << std::endl;

  // Order of initialization is important in some cases!

  initializeGeometry();
  std::cout << "Geometry initialized." << std::endl;

  initializeShaderPrograms();
  std::cout << "Shader programs initialized." << std::endl;

  if (initializeUniformBuffers())
  { std::cout << "Uniform buffers initialized." << std::endl; }
  else { throw std::logic_error("Failed to initialize uniform buffers!"); }

  initializeStars();
  std::cout << "Stars initialized." << std::endl;

  initializePlanets();
  std::cout << "Planets initialized." << std::endl;

  initializeSkybox();
  std::cout << "Skybox initialized." << std::endl;

  if (useFrameBuffer)
  {
    if (initializeFrameBuffer())
    {
      std::cout << "FrameBuffer initialized." << std::endl;
    }
    else
    {
      std::cout << "FrameBuffer initialization failed!" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0); // use default framebuffer
      useFrameBuffer = false;
    }
  }
  else
  {
    std::cout << "FrameBuffer disabled." << std::endl;
  }

  if (!renderTexts)
  { std::cout << "Rendering text is disabled." << std::endl; }
  else
  {
    if (initializeFonts())
    {
      std::cout << "TextLoader initialized." << std::endl;
      initializeTexts(*textLoader);
      std::cout << texts.size() << " texts initialized." << std::endl;
    }
    else
    {
      std::cout << "Failed to initialize TextLoader!" << std::endl;
      renderTexts = false; // not possible to render text if this case occurs
    }
  }
}


bool ApplicationSolar::useShader(std::string shaderName) const
{
  if (lastProg != shaderName)
  {
    glUseProgram(m_shaders.at(shaderName).handle);
    lastProg = shaderName;
    return true;
  }
  return false;
}


void ApplicationSolar::renderObject(std::shared_ptr<Planet> planet) const
{
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
    useShader("sun");

    // we would prefer to pass a vector here but the assignment tells us to use Uniform3f
    glUniform3f(m_shaders.at("sun").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);

    // upload model matrix to shader
    glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface - currently not needed
    glm::fmat4 normal_matrix = glm::inverseTranspose(view_matrix * model_matrix);
    glUniformMatrix4fv(m_shaders.at("sun").u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));


    // =============== TEXTURING =============== //

    bool hasTex = planet->hasTexture() && useTextures;

    if (hasTex)
    {
      // get the uniform location for the texture from the shader
      GLuint sampler_loc = m_shaders.at("sun").u_locs.at("tex");

      // use TEXTURE0 and thus, upload 0 at glUniform1i as well
      texture_info texinf = planet->getTextureInfo();
      glActiveTexture(GL_TEXTURE0 + texinf.unit);
      glBindTexture(GL_TEXTURE_2D, loaded_textures.at(texinf.index));
      glUniform1i(sampler_loc, texinf.unit);
    }

    // upload information whether or not to use the texture
    glUniform1i(m_shaders.at("sun").u_locs.at("useTexture"), hasTex);

    // ========================================= //


    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    //glEnable(GL_DEPTH_TEST); // already enabled by launcher.cpp render function
    glEnable(GL_CULL_FACE);  // enable culling so we don't render back faces
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
  else // render planets and moons
  {
    if (shadingMode == 0) // blinn phong shading
    {
      if (useShader("planet"))
      {
        glEnable(GL_CULL_FACE);  // enable culling
        glCullFace(GL_BACK);     // cull back faces so we don't render back faces
      }

      // we would prefer to pass a vector here but the assignment tells us to use Uniform3f
      glUniform3f(m_shaders.at("planet").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);



      // =============== TEXTURING =============== //

      bool hasTex = planet->hasTexture() && useTextures;

      // texture flags telling which textures to use
      unsigned char texflags = 0;

      if (hasTex)
      {
        // get the uniform location for the texture from the shader
        GLuint sampler_loc = m_shaders.at("planet").u_locs.at("tex");
        texture_info texinf = planet->getTextureInfo(); // use diffuse texture
        uploadTexture(sampler_loc, texinf);


        // check if the planet has a night texture to use for the dark areas of the planet
        if (planet->hasTexture("night"))
        {
          texinf = planet->getTextureInfo("night");

          // upload texture ID to sampler location
          sampler_loc = glGetUniformLocation(m_shaders.at("planet").handle, "tex_night");
          uploadTexture(sampler_loc, texinf);

          // tell shader that we got a night texture to use
          texflags |= FLAG_TEX_NIGHT;
        }


        // check if the planet has a normal map texture, if so then use it - otherwise don't
        if (planet->hasTexture("normal") && useNormalMapping)
        {
          texinf = planet->getTextureInfo("normal");

          // upload texture ID to sampler location
          sampler_loc = glGetUniformLocation(m_shaders.at("planet").handle, "tex_normal");
          uploadTexture(sampler_loc, texinf);

          // upload normal factor
          sampler_loc = glGetUniformLocation(m_shaders.at("planet").handle, "factor_normal");
          glUniform1f(sampler_loc, texinf.factor);

          // tell shader that we got a normal texture to use
          texflags |= FLAG_TEX_NORMAL;
        }


        // check if the planet has a specular map texture, if so then use it - otherwise don't
        if (planet->hasTexture("specular"))
        {
          texinf = planet->getTextureInfo("specular");

          // upload texture ID to sampler location
          sampler_loc = glGetUniformLocation(m_shaders.at("planet").handle, "tex_specular");
          uploadTexture(sampler_loc, texinf);

          // tell shader that we got a specular texture to use
          texflags |= FLAG_TEX_SPECULAR;
        }
      }

      // upload information whether or not to use the texture
      glUniform1i(m_shaders.at("planet").u_locs.at("useTexture"), hasTex);

      // using this method, we only upload one uniform for the whole active texture information
      glUniform1i(m_shaders.at("planet").u_locs.at("texture_flags"), texflags);

      // =============== END OF TEXTURING =============== //



      // upload model matrix to shader
      glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

      // extra matrix for normal transformation to keep them orthogonal to surface - currently not needed
      //glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
      glm::fmat4 normal_matrix = glm::inverseTranspose(view_matrix * model_matrix);
      glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"), 1, GL_FALSE, glm::value_ptr(normal_matrix));

      // bind the VAO to draw
      glBindVertexArray(planet_object.vertex_AO);

      // draw bound vertex array using bound shader
      //glEnable(GL_DEPTH_TEST); // already enabled by launcher.cpp render function
      glEnable(GL_CULL_FACE);
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
    }
    else if (shadingMode == 1) // cel shading
    {
      // bind the VAO to draw
      glBindVertexArray(planet_object.vertex_AO);

      // 1. draw the silhouette
      useShader("celSilhouette");
      glUniform3fv(m_shaders.at("celSilhouette").u_locs.at("BorderColor"), 1, borderColor.data());
      glUniformMatrix4fv(m_shaders.at("celSilhouette").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));
  
      // draw the object just 2d with the border color
      glEnable(GL_CULL_FACE); // enable culling
      glCullFace(GL_FRONT);   // enable culling of front faces (render back faces)
      //glDepthMask(GL_TRUE); // enable writes to Z-buffer (needed for texture cel shading later)
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);

      // 2. draw the cel shading color
      useShader("celColor");
      glUniform3f(m_shaders.at("celColor").u_locs.at("Color"), planetColor[0], planetColor[1], planetColor[2]);
      glUniformMatrix4fv(m_shaders.at("celColor").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(model_matrix));

      // draw the 3d object layer and add it to the 2d border layer
      glCullFace(GL_BACK);     // enable culling of back faces (don't render them)
      //glDepthMask(GL_FALSE); // disable writes to Z-buffer (needed for texture cel shading later)
      glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
    }
  }

  // render the planets 3D text
  lastProg = ""; // set last prog to another so shader will be changed after this loop
  renderPlanetText(planet, model_matrix);
}


void ApplicationSolar::renderOrbits(std::shared_ptr<Planet> planet) const
{
  if (!showOrbits) { return; }

  std::deque<std::shared_ptr<Planet>> moons = planet->getMoons();
  std::shared_ptr<Planet> moon;
  for (unsigned int i = 0; i < moons.size(); i++)
  {
    moon = moons.at(i);
    renderOrbits(moon);
  }

  if (planet->getOrbitPointCount() <= 0) { return; }

  // RENDER ORBITS
  useShader("orbits");
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
  useShader("stars");

  // bind the VAO to draw, set the point size and draw
  glBindVertexArray(stars.vertex_AO);
  glPointSize(starSize);
  glDrawArrays(stars.draw_mode, 0, starCount);
}


void ApplicationSolar::renderSkybox() const
{
  if (!showSkybox) { return; }

  // the case that the cubemap failed loading
  if (skyboxTexture.index < 0) { return; }


  // use skybox shader program
  useShader("skybox");

  // bind the VAO to draw
  glBindVertexArray(skybox_object.vertex_AO);

  glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ModelMatrix"), 1, GL_FALSE, glm::value_ptr(skyboxModelMatrix));

  // apply cubemap texture
  GLuint sampler_loc = m_shaders.at("skybox").u_locs.at("Skybox");
      
  // bind the correct cubemap texture for usage
  glActiveTexture(GL_TEXTURE0 + skyboxTexture.unit);
  glBindTexture(GL_TEXTURE_CUBE_MAP, loaded_textures.at(skyboxTexture.index));
  glUniform1i(sampler_loc, skyboxTexture.unit);

  // draw bound vertex array using bound shader
  glEnable(GL_CULL_FACE);   // enable culling
  glCullFace(GL_FRONT);     // enable culling of front faces (render back faces)
  glDisable(GL_DEPTH_TEST);
  glDrawElements(skybox_object.draw_mode, skybox_object.num_elements, model::INDEX.type, NULL);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}


void ApplicationSolar::renderText() const
{
  // a little test section for the GUI-text (let it show the runtime and move)
  double progTime = glfwGetTime();
  std::string outTime = std::to_string(glm::round(progTime));
  outTime.erase(outTime.find_last_not_of('0'), std::string::npos); // remove trailing zeros
  texts2D.at(0)->setText("Runtime: " + outTime + " sec");
  texts2D.at(0)->setPosition(25.f + glm::cos(progTime) * 20, 25.f + glm::sin(progTime) * 20);

  // render gui (2D) texts
  for (auto& text : texts2D)
  {
    text->render(m_shaders.at("text2D").handle);
  }
}


void ApplicationSolar::renderPlanetText(std::shared_ptr<Planet> planet, glm::fmat4& modelMatrix) const
{
  std::shared_ptr<Text3D> t3D = planet->get3DText();

  if (t3D != nullptr)
  {
    //glm::fmat4 modelMatrix = planet->getModelMatrix();
    modelMatrix = glm::translate(modelMatrix, t3D->getPosition());
    //modelMatrix = glm::scale(modelMat, glm::fvec3(0.5f));
    t3D->setModelMatrix(modelMatrix);
    t3D->render(m_shaders.at("text3D").handle, 0.05f);
  }
}


void ApplicationSolar::render() const
{
  // bind the framebuffer to use it (draw to it)
  if (useFrameBuffer)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

    // clear the framebuffer first (use color black)
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // scene rendering
  renderSkybox();    // render the skybox
  renderStars();     // render all stars
  renderObject(sun); // render the sun and all their planets with their moons
  renderOrbits(sun); // render the orbits of all the planets and the moons
  if (renderTexts) { renderText(); } // render all the texts

  // draw the framebuffer using the default framebuffer to the screen
  if (useFrameBuffer) { frameBufferDrawing(); }

  // We sadly have to use a mutable here because the
  // framework does not provide a main loop and
  // we don't want to change it too much.
  double currentTimestamp = glfwGetTime();
  if (lastTimestamp < 0.0) { lastTimestamp = currentTimestamp; }
  deltaTime = float(currentTimestamp - lastTimestamp);
  lastTimestamp = currentTimestamp;
}


void ApplicationSolar::frameBufferDrawing() const
{
  // bind default framebuffer for on-screen drawing
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // bind screen quad to draw
  glBindVertexArray(screenQuad.vertex_AO);

  // we dont need depth test because we only want to see the quad
  glDisable(GL_DEPTH_TEST);

  // use the on-screen shader program
  useShader("screen");

  // upload effect uniforms
  glUniform1i(m_shaders.at("screen").u_locs.at("effectFlags"), screenEffects);

  // use texture unit 0 to draw and bind the texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

  // draw the screen quad (6 indices)
  glDrawArrays(GL_TRIANGLES, 0, 6);
}



// update view matrix of all shader programs
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


  // upload view matrix to GPU
  for (const std::string& prog : updateViewFor)
  {
    glUseProgram(m_shaders.at(prog).handle);
    glUniformMatrix4fv(m_shaders.at(prog).u_locs.at("ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_matrix));
  }

  lastProg = ""; // so that next time a program needs to be used anyway


  // (assignment 6) change data of CameraBuffer struct and update the related GPU buffer
  camBuffer.ViewMatrix = view_matrix;
  updateUniformBuffer(UBO_camera, &camBuffer, sizeof(camBuffer));
}


// update projection matrix of all shader programs
void ApplicationSolar::updateProjection()
{
  // upload projection matrix to gpu
  std::vector<std::string> progs;
  //progs.push_back("planet"); // done by UBO (since assignment 6)
  //progs.push_back("sun");
  progs.push_back("stars");
  progs.push_back("orbits");
  progs.push_back("celSilhouette");
  progs.push_back("celColor");
  progs.push_back("skybox");


  // upload the matrix to the shaders
  for (const std::string& prog : progs)
  {
    glUseProgram(m_shaders.at(prog).handle);
    glUniformMatrix4fv(m_shaders.at(prog).u_locs.at("ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(m_view_projection));
  }

  lastProg = ""; // so that next time a program needs to be used anyway


  // (assignment 6) change data of CameraBuffer struct and update the related GPU buffer
  camBuffer.ProjectionMatrix = m_view_projection;
  updateUniformBuffer(UBO_camera, &camBuffer, sizeof(camBuffer));
}


// update uniform locations
void ApplicationSolar::uploadUniforms()
{
  updateUniformLocations();
  
  // query the uniform block index from all shaders that use a uniform buffer object (assignment 6)
  queryUniformBlockIndex(UBO_camera_bi, "planet", "camera_data");
  queryUniformBlockIndex(UBO_camera_bi, "sun", "camera_data");
  queryUniformBlockIndex(UBO_camera_bi, "text3D", "camera_data");

  updateView();
  updateProjection();

  // upload kernel for screen shader
  useShader("screen");
  glUniformMatrix3fv(m_shaders.at("screen").u_locs.at("Kernel3x3"), 1, GL_FALSE, glm::value_ptr(blurKernel));
}


// uploads texture uniforms
void ApplicationSolar::uploadTexture(GLuint sampler_location, texture_info texinf) const
{
  if (texinf.index < 0) { return; } // invalid texture info
  glActiveTexture(GL_TEXTURE0 + texinf.unit);
  glBindTexture(GL_TEXTURE_2D, loaded_textures.at(texinf.index));
  glUniform1i(sampler_location, texinf.unit);
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

      case GLFW_KEY_1: // to use blinn phong shading mode
        if (shadingMode == 0 || !active) { break; }
        std::cout << "Using Blinn Phong Shading now." << std::endl;
        shadingMode = 0;
        break;

      case GLFW_KEY_2: // to use cel shading mode
        if (shadingMode == 1 || !active) { break; }
        std::cout << "Using Cel Shading now." << std::endl;
        shadingMode = 1;
        break;

      case GLFW_KEY_T: // to enable/disable textures
        if (active) {
          useTextures = !useTextures;
          std::cout << "Textures " << (useTextures ? "enabled" : "disabled") << std::endl;
        }
        break;

      case GLFW_KEY_N: // to enable/disable normal mapping
        if (active) {
          useNormalMapping = !useNormalMapping;
          std::cout << "Normal mapping " << (useNormalMapping ? "enabled" : "disabled") << std::endl;
        }
        break;

      case GLFW_KEY_O: // to enable/disable orbits
        if (active) {
          showOrbits = !showOrbits;
          std::cout << "Orbits " << (showOrbits ? "enabled" : "disabled") << std::endl;
        }
        break;

      case GLFW_KEY_7: // toggle grayscale effect
        if (active) { toggleEffect(screenEffects, FLAG_EFFECT_GRAYSCALE, "grayscale"); }
        break;

      case GLFW_KEY_8: // toggle horizontal mirrored effect
        if (active) { toggleEffect(screenEffects, FLAG_EFFECT_MIRRORED_HORIZONTAL, "horizontal mirrored"); }
        break;

      case GLFW_KEY_9: // toggle vertical mirrored effect
        if (active) { toggleEffect(screenEffects, FLAG_EFFECT_MIRRORED_VERTICAL, "vertical mirrored"); }
        break;

      case GLFW_KEY_0: // toggle blurred image effect
        if (active) { toggleEffect(screenEffects, FLAG_EFFECT_BLURRED, "blurred"); }
        break;

      case GLFW_KEY_P: // [P]rint debug - to check camera position and view matrix
        if (active)
        {
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
        }
        break;
    }
  }

  move();
}


// to enable/disable specific effects
void ApplicationSolar::toggleEffect(unsigned char& effectFlags, const int effectFlag, std::string name)
{
  screenEffects = screenEffects ^ effectFlag;
  bool state = screenEffects & effectFlag;
  std::cout << "Effect (" << name << ") " << (state ? "enabled" : "disabled") << std::endl;
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

    movementVector *= deltaTime;

    updateView();
  }
}


// handle delta mouse movement input
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
  m_shaders.emplace("planet", shader_program{m_resource_path + "shaders/planet_viewspace.vert",
                                             m_resource_path + "shaders/planet_viewspace.frag"});

  m_shaders.emplace("sun", shader_program{m_resource_path + "shaders/sun.vert",
                                          m_resource_path + "shaders/sun.frag"});

  // request uniform locations for planet shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  //m_shaders.at("planet").u_locs["ViewMatrix"] = -1; // done by UBO (since assignment 6)
  //m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["Color"] = -1;
  m_shaders.at("planet").u_locs["useTexture"] = -1;
  m_shaders.at("planet").u_locs["tex"] = -1;
  m_shaders.at("planet").u_locs["texture_flags"] = -1;

  // request uniform locations for sun shader program
  m_shaders.at("sun").u_locs["NormalMatrix"] = -1;
  m_shaders.at("sun").u_locs["ModelMatrix"] = -1;
  //m_shaders.at("sun").u_locs["ViewMatrix"] = -1; // done by UBO (since assignment 6)
  //m_shaders.at("sun").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("sun").u_locs["Color"] = -1;
  m_shaders.at("sun").u_locs["useTexture"] = -1;
  m_shaders.at("sun").u_locs["tex"] = -1;

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

  m_shaders.at("stars").u_locs["ViewMatrix"] = -1;
  m_shaders.at("stars").u_locs["ProjectionMatrix"] = -1;



  // shader for orbits
  m_shaders.emplace("orbits", shader_program{m_resource_path + "shaders/orbits.vert",
                                             m_resource_path + "shaders/orbits.frag"});

  m_shaders.at("orbits").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbits").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbits").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("orbits").u_locs["OrbitColor"] = -1;



  // shader for skybox (assignment 4)
  m_shaders.emplace("skybox", shader_program{m_resource_path + "shaders/skybox.vert",
                                             m_resource_path + "shaders/skybox.frag"});

  m_shaders.at("skybox").u_locs["ModelMatrix"] = -1;
  m_shaders.at("skybox").u_locs["ViewMatrix"] = -1;
  m_shaders.at("skybox").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("skybox").u_locs["Skybox"] = -1;


  // shader for framebuffer rendering
  m_shaders.emplace("screen", shader_program{m_resource_path + "shaders/screen.vert",
                                             m_resource_path + "shaders/screen.frag" });

  m_shaders.at("screen").u_locs["frameBufferTex"] = -1;
  m_shaders.at("screen").u_locs["effectFlags"] = -1;
  m_shaders.at("screen").u_locs["Kernel3x3"] = -1;


  // 2D text shader
  m_shaders.emplace("text2D", shader_program{m_resource_path + "shaders/text2D.vert",
                                           m_resource_path + "shaders/text2D.frag"});


  // 3D text shader
  m_shaders.emplace("text3D", shader_program{m_resource_path + "shaders/text3D.vert",
                                             m_resource_path + "shaders/text3D.frag"});


  // register programs to which the view matrix should be uploaded after changes
  //updateViewFor.push_back("planet"); // done by UBO (since assignment 6)
  //updateViewFor.push_back("sun");
  updateViewFor.push_back("stars");
  updateViewFor.push_back("orbits");
  updateViewFor.push_back("celSilhouette");
  updateViewFor.push_back("celColor");
  updateViewFor.push_back("skybox");
}


// load models
void ApplicationSolar::initializeGeometry()
{  
  model planet_model = model_loader::obj(m_resource_path + "models/sphere_own_uv.obj", model::NORMAL | model::TEXCOORD | model::TANGENT);

  // more models (not all contain uv mapping)
  //model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL | model::TEXCOORD | model::TANGENT);
  //model planet_model = model_loader::obj(m_resource_path + "models/sphere_own_fromcube_uv.obj", model::NORMAL | model::TEXCOORD | model::TANGENT);
  //model planet_model = model_loader::obj(m_resource_path + "models/test_cube.obj", model::NORMAL);
  //model planet_model = model_loader::obj(m_resource_path + "models/test_monkey.obj", model::NORMAL);
  //model planet_model = model_loader::obj(m_resource_path + "models/fidget_01.obj", model::NORMAL);


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

  // activate third attribute on gpu
  glEnableVertexAttribArray(2);
  // attribute for uv mapping
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);

  // activate attribute on gpu
  glEnableVertexAttribArray(3);
  // attribute for normal mapping
  glVertexAttribPointer(3, model::TANGENT.components, model::TANGENT.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TANGENT]);


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


// generate stars
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

  std::shared_ptr<Planet> neptune = std::make_shared<Planet>("Neptune", size_uranus, ot_neptune, dt_neptune, glm::fvec3{}, glm::fvec3{ 0.0f, 0.0f, dist_neptune });
  neptune->setColor(100, 120, 200);


  // set planet texture path
  std::string texPath = m_resource_path + "textures/planet/";
  mercury->setTexture(loadTexture(texPath + "mercury.png"));
  venus->setTexture(loadTexture(texPath + "venus.png"));

  // texture from https://www.solarsystemscope.com/textures
  earth->setTexture(loadTexture(texPath + "earth_daymap.png"));
  earth->setTexture(loadTexture(texPath + "earth_nightmap.png", 1), "night");
  earth->setTexture(loadTexture(texPath + "earth_normalmap.png", 2), "normal", 6.0f);
  earth->setTexture(loadTexture(texPath + "earth_specularmap.png", 3), "specular");

  moon1->setTexture(loadTexture(texPath + "moon.png"));
  moon1->setTexture(loadTexture(texPath + "moon_normalmap.png", 2), "normal");

  mars->setTexture(loadTexture(texPath + "mars.png"));
  jupiter->setTexture(loadTexture(texPath + "jupiter.png"));
  saturn->setTexture(loadTexture(texPath + "saturn.png"));
  uranus->setTexture(loadTexture(texPath + "uranus.png"));
  neptune->setTexture(loadTexture(texPath + "neptune.png"));

  sun->setTexture(loadTexture(texPath + "sun.png"));


  // add colors to list of planets that we want to render
  sun->addMoon(mercury);
  sun->addMoon(venus);
  sun->addMoon(earth);
  sun->addMoon(mars);
  sun->addMoon(jupiter);
  sun->addMoon(saturn);
  sun->addMoon(uranus);
  sun->addMoon(neptune);


  // add planet 3D text
  Text3D text1{"Earth", nullptr, glm::fvec3{0.f, 10.f, 0.f}, glm::fvec3{1.0f}, winWidth, winHeight};
  std::shared_ptr<Text3D> t1 = std::make_shared<Text3D>(text1);
  
  earth->set3DText(t1);
  texts3D.push_back(t1);


  // test debug
  std::cout << "(i) Sun has a size of " << size_sun << std::endl;
  std::cout << "(i) Rotation angle for \"" << sun->getName() << "\" per second is: " << sun->getRotationAngle(time_multiplier) << std::endl;
  std::cout << "(i) Textures loaded: " << loaded_textures.size() << std::endl;
}


// Loads a texture and returns the texture information.
// (index = index in loaded textures vector!)
// Returns texture_info with "index = -1" if loading failed!
texture_info ApplicationSolar::loadTexture(const std::string& path, int textureUnit)
{
  // Part of the stbi documentation, but we can use the given texture_loader instead.
  // int x,y,n;
  // unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
  // // ... process data if not NULL ...
  // // ... x = width, y = height, n = # 8-bit components per pixel ...
  // // ... replace '0' with '1'..'4' to force that many components per pixel
  // // ... but 'n' will always be the number that it would have been if you said 0
  // stbi_image_free(data)

  // try to load the texture by the given path
  // and catch all possible exceptions
  try
  {
    pixel_data image = texture_loader::file(path);

    GLuint texID;
    glGenTextures(1, &texID);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D,       // target
                 0,                   // level (level-of-detail number) (mipmap reduction)
                 image.channels,      // internalFormat (number of color components)
                 image.width,         // width
                 image.height,        // height
                 0,                   // border (must be 0)
                 image.channels,      // format of the pixel data
                 image.channel_type,  // type (data type of pixel data) (GL_UNSIGNED_BYTE...)
                 image.ptr());        // data (pointer to the image data in memory)

    // use mipmap generation and create mipmaps
    glGenerateTextureMipmap(texID);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // add to loaded textures vector
    loaded_textures.push_back(texID);

    std::cout << "Successfully loaded texture \"" + path + "\"." << std::endl;

    // return where the texID is stored in the vector (the index)
    return texture_info{(int) loaded_textures.size() - 1, textureUnit, path};
  }
  catch (const std::exception& e)
  {
    std::cerr << "Loading texture \"" << path << "\" failed!\n"
              << e.what() << std::endl;
  }

  return texture_info{-1, textureUnit, path};
}


// Loads a cubemap and returns the texture information.
// Returns texture_info with "index = -1" if loading failed!
texture_info ApplicationSolar::loadCubemap(const std::vector<std::string>& paths, int textureUnit)
{
  std::string lastPathTried = "none";

  try
  {
    GLuint texID;
    glGenTextures(1, &texID);

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    // load all textures
    int i = 0;
    for (const std::string& path : paths)
    {
      lastPathTried = path;
      pixel_data image = texture_loader::file(path);

      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i++,  // target
                   0,                   // level (level-of-detail number) (mipmap reduction)
                   image.channels,      // internalFormat (number of color components)
                   image.width,         // width
                   image.height,        // height
                   0,                   // border (must be 0)
                   image.channels,      // format of the pixel data
                   image.channel_type,  // type (data type of pixel data) (GL_UNSIGNED_BYTE...)
                   image.ptr());        // data (pointer to the image data in memory)
    }

    // set texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // add to loaded textures vector
    loaded_textures.push_back(texID);

    std::cout << "Successfully loaded cubemap." << std::endl;

    // return where the texID is stored in the vector (the index)
    return texture_info{(int) loaded_textures.size() - 1, textureUnit, paths};
  }
  catch (const std::exception& e)
  {
    std::cerr << "Loading cubemap failed!\n"
              << ">> Last path tried: " << lastPathTried << "\n"
              << ">> " << e.what() << std::endl;
  }

  return texture_info{-1, textureUnit, paths};
}


void ApplicationSolar::initializeSkybox()
{
  model skybox_model = model_loader::obj(m_resource_path + "models/skybox.obj");

  // generate vertex array object and bind it for attaching buffers
  glGenVertexArrays(1, &skybox_object.vertex_AO);
  glBindVertexArray(skybox_object.vertex_AO);

  // generate vertex buffer object and bind it
  glGenBuffers(1, &skybox_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, skybox_object.vertex_BO);

  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * skybox_model.data.size(), skybox_model.data.data(), GL_STATIC_DRAW);

  // attributes
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, skybox_model.vertex_bytes, skybox_model.offsets[model::POSITION]);

  // generate element buffer object and bind it
  glGenBuffers(1, &skybox_object.element_BO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_object.element_BO);

  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * skybox_model.indices.size(), skybox_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  skybox_object.draw_mode = GL_TRIANGLES;

  // transfer number of indices to model object 
  skybox_object.num_elements = GLsizei(skybox_model.indices.size());

  // load skybox textures (order refers to the opengl specification for cubemaps!)
  std::vector<std::string> skyboxTexturePaths;
  std::string folder = "textures/skybox/stars_02/";
  std::string ext = ".png";
  skyboxTexturePaths.push_back(m_resource_path + folder + "right" + ext);   // +x
  skyboxTexturePaths.push_back(m_resource_path + folder + "left" + ext);    // -x
  skyboxTexturePaths.push_back(m_resource_path + folder + "bottom" + ext);  // +y
  skyboxTexturePaths.push_back(m_resource_path + folder + "top" + ext);     // -y
  skyboxTexturePaths.push_back(m_resource_path + folder + "back" + ext);    // +z
  skyboxTexturePaths.push_back(m_resource_path + folder + "front" + ext);   // -z

  // create the cubemap using the paths from above
  skyboxTexture = loadCubemap(skyboxTexturePaths);
}


// intializes the framebuffer
bool ApplicationSolar::initializeFrameBuffer()
{
  // create the quad for rendering the framebuffer
  // create the quad vertex array object
  GLuint screenQuad_AO;
  glGenVertexArrays(1, &screenQuad_AO);
  glBindVertexArray(screenQuad_AO);

  // quad vertex buffer data
  // we could also use indexing but its not needed here
  static const GLfloat quad_vbd[] =
  {
    -1.0f, -1.0f, 0.0f, /* v1 */ 0.0f, 0.0f, /* v1_uv */
     1.0f, -1.0f, 0.0f, /* v2 */ 1.0f, 0.0f, /* v2_uv */
    -1.0f,  1.0f, 0.0f, /* v4 */ 0.0f, 1.0f, /* v4_uv */
    -1.0f,  1.0f, 0.0f, /* v4 */ 0.0f, 1.0f, /* v4_uv */
     1.0f, -1.0f, 0.0f, /* v2 */ 1.0f, 0.0f, /* v2_uv */
     1.0f,  1.0f, 0.0f, /* v3 */ 1.0f, 1.0f, /* v3_uv */
  };

  // create the quad vertex buffer object
  GLuint screenQuad_BO;
  glGenBuffers(1, &screenQuad_BO);
  glBindBuffer(GL_ARRAY_BUFFER, screenQuad_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vbd), quad_vbd, GL_STATIC_DRAW);

  // activate first attribute on gpu (position of the vertex)
  glEnableVertexAttribArray(0);
  // one vertex consists of 3 floats for position and 2 for UV (= stride of 5)
  glVertexAttribPointer(0, 3, model::POSITION.type, GL_FALSE, 5 * sizeof(float), 0);

  // activate first attribute on gpu (position of the vertex)
  glEnableVertexAttribArray(1);
  // one vertex consists of 3 floats for position and 2 for UV (= stride of 5)
  glVertexAttribPointer(1, 2, model::POSITION.type, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

  screenQuad.vertex_AO = screenQuad_AO;
  screenQuad.vertex_BO = screenQuad_BO;
  // because we create the quad out of 2 triangles (each with 3 vertices)
  screenQuad.num_elements = 2;


  // create the framebuffer
  glGenFramebuffers(1, &frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

  // generate texture and attach it
  glGenTextures(1, &frameBufferTexture);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);

  // create texture without data (empty)
  glTexImage2D
  (
    GL_TEXTURE_2D,    // target
    0,                // level (level-of-detail number) (mipmap reduction)
    GL_RGB,           // internalFormat (number of color components)
    winWidth,         // width
    winHeight,        // height
    0,                // border (must be 0)
    GL_RGB,           // format of the pixel data
    GL_UNSIGNED_BYTE, // type (data type of pixel data) (GL_UNSIGNED_BYTE...)
    NULL              // data (pointer to the image data in memory)
  );
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


  // generate and bind render buffer object
  glGenRenderbuffers(1, &depthRenderBuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, winWidth, winHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);


  // set the texture as colour attachement 0
  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, frameBufferTexture, 0);

  // set the list of draw buffers
  GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
  int draw_buffers_size = sizeof(draw_buffers) / sizeof(draw_buffers[0]);

  // render outputs from fragment shaders to these buffers
  glDrawBuffers(draw_buffers_size, draw_buffers);

  // check that the framebuffer is "ok"
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  { return false; }
  
  return true;
}


// initialize uniform buffer objects
// Returns false if this operation is not possible, true otherwise.
bool ApplicationSolar::initializeUniformBuffers()
{
  GLint gpu_maxBindingPoints;
  glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &gpu_maxBindingPoints);

  // check that the gpu supports the needed amount of binding points
  if (gpu_maxBindingPoints < gpu_minBindingPoints)
  {
    std::cerr << "GPU does not have enough binding points (" << gpu_minBindingPoints << ")!" << std::endl;
    return false;
  }

  // Create Camera UNIFORM BUFFER OBJECT
  glGenBuffers(1, &UBO_camera);
  glBindBufferBase(GL_UNIFORM_BUFFER, UBO_camera_bi, UBO_camera);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(camBuffer), &camBuffer, GL_DYNAMIC_DRAW);

  return true;
}


// Query the shaders uniform block index location and upload the global binding point value to it
void ApplicationSolar::queryUniformBlockIndex(GLuint blockIndex, std::string shaderName, char* uniformName)
{
  // query block index location
  // blockIndexLocation = shader location of the "uniform block index"
  GLuint blockIndexLocation = glGetUniformBlockIndex(m_shaders.at(shaderName).handle, uniformName);

  // upload global binding point value to the shaders block index location
  glUniformBlockBinding(m_shaders.at(shaderName).handle, blockIndexLocation, blockIndex);
}


// Update the data of the camera uniform buffer
void ApplicationSolar::updateUniformBuffer(GLuint bufferHandle, void* sourceData, size_t sourceDataSize)
{
  glBindBuffer(GL_UNIFORM_BUFFER, bufferHandle);
  void* buffer_ptr = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
  // copy the data from src=camBuffer to dst=buffer_ptr
  std::memcpy(buffer_ptr, sourceData, sourceDataSize);
  glUnmapBuffer(GL_UNIFORM_BUFFER);
}


bool ApplicationSolar::initializeFonts()
{
  textLoader = std::make_shared<TextLoader>();
  textLoader->addFont("font1", m_resource_path + "fonts/source-code-pro/SourceCodePro-Regular.ttf", 32);
  textLoader->addFont("font2", m_resource_path + "fonts/source-code-pro/SourceCodePro-Black.ttf", 20);
  if (!textLoader->load()) { return false; }
  return true;
}


void ApplicationSolar::initializeTexts(TextLoader& tl)
{
  if (tl.hasFont("font1"))
  {
    Text2D test1{"Moving 2D Text!", std::make_shared<Font>(tl.getFont("font1")), glm::fvec2{25.f}, glm::ivec3{1.0f, 0.8f, 0.3f}, winWidth, winHeight};
    texts2D.push_back(std::make_shared<Text2D>(test1));
  }
  else { std::cout << "MISSING FONT 1!" << std::endl; }

  if (tl.hasFont("font2"))
  {
    std::shared_ptr<Font> font2 = std::make_shared<Font>(tl.getFont("font2"));
    Text2D test2{"Static 2D Text", font2, glm::fvec2{(float) winWidth - 200.f, (float) winHeight - 50.f}, glm::ivec3{ 0.2f, 0.2f, 1.0f }, winWidth, winHeight};
    texts2D.push_back(std::make_shared<Text2D>(test2));

    // set the font for the planet texts
    for (auto& text : texts3D)
    { text->setFont(font2); }
  }
  else { std::cout << "MISSING FONT 2!" << std::endl; }

  // add the different vectors to the vector containing all types of text
  texts.insert(texts.end(), texts2D.begin(), texts2D.end());
  texts.insert(texts.end(), texts3D.begin(), texts3D.end());
}


ApplicationSolar::~ApplicationSolar()
{
  std::cout << "Closing Application...\nCleaning up resources..." << std::endl;

  // clean up loaded textures
  glDeleteTextures(loaded_textures.size(), loaded_textures.data());
  glDeleteTextures(1, &frameBufferTexture);

  // clean up text character textures for all fonts
  textLoader->cleanupFontTextures();

  // planets
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  
  // stars
  glDeleteBuffers(1, &stars.vertex_BO);
  glDeleteVertexArrays(1, &stars.vertex_AO);

  // skybox
  glDeleteBuffers(1, &skybox_object.vertex_BO);
  glDeleteBuffers(1, &skybox_object.element_BO);
  glDeleteVertexArrays(1, &skybox_object.vertex_AO);

  // clear framebuffers and renderbuffers
  glDeleteFramebuffers(1, &frameBuffer);
  glDeleteRenderbuffers(1, &depthRenderBuffer);

  // clean up UBOs
  glDeleteBuffers(1, &UBO_camera);

  std::cout << "Finished cleanup.\nExit." << std::endl;
}


// exe entry point
int main(int argc, char* argv[])
{
  Launcher::run<ApplicationSolar>(argc, argv);
}