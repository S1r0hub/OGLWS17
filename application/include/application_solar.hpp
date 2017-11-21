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

// added for assignment 4
#include "texture_info.hpp"


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
    void renderObject(std::shared_ptr<Planet> planet) const;

    // render the orbits of planets and moons
    void renderOrbits(std::shared_ptr<Planet> planet) const;

    // render stars and upload information to gpu
    void renderStars() const;

    // render the skybox
    void renderSkybox() const;

    // uses this shader and remembers the last one
    // returns true if the shader state changed
    bool useShader(std::string shaderName) const;


  protected:

    void initializeShaderPrograms();
    void initializeGeometry();
    void initializePlanets();
    void initializeStars();
    void initializeSkybox();
    void updateView();
    void move();

    // cpu representation of model
    model_object planet_object;
    model_object stars{};
    model_object skybox_object;


  private:

    // simulation speed used by planets (1 = real time) (use around 1 million)
    double time_multiplier = 10000;

    // deque holding pointers for the planet and sun objects
    std::shared_ptr<Planet> sun;

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
    float cameraSpeed = 30.f;
    float cameraRotationSpeed = 3.f;

    // store information about the camera rotation around the x-axis
    float cameraRotationX = 0.f;
    float cameraRotationX_max = MATH_HALF_PI;
    float cameraRotationX_min = -MATH_HALF_PI;

    // how many times faster the camera moves on sprinting
    float cameraSprintMultiplier = 2.0f;

    // store information about how the movement should be done
    glm::fvec3 movementVector{0.f, 2.f, 30.f}; // initial camera position

    unsigned int starCount = 2000;
    float starSize = 1.f;
    int starDistance_min = 45;
    int starDistance_max = 200;

    glm::mat4 view_matrix{};

    // 0 = blinn phong, 1 = cel shading
    int shadingMode = 0;
    std::vector<float> borderColor = {1.0, 1.0, 1.0};

    // delta time related
    // mutable because we have to change it in the const render function
    mutable float deltaTime = 0.0;
    mutable double lastTimestamp = -1.0;


    // vector that holds the name of shader programs to which the view matrix should be uploaded
    std::vector<std::string> updateViewFor;


    // whether or not to use textures (can be changed by user pressing the key <T>)
    bool useTextures = true;

    // Loads a texture and returns the texture unit ID.
    // (ID = Index in loaded textures vector!)
    // Will also add the texture unit ID to the list so
    // all the textures will be deleted on destructor call.
    // Returns -1 if loading the texture failed!
    texture_info loadTexture(const std::string& path, int textureUnit = 0);
    std::vector<GLuint> loaded_textures;

    // to reduce multiple calls of glUseProgram when the same program was loaded before
    mutable std::string lastProg = "";

    // hold information like index of the textures stored in loaded_textures
    bool showSkybox = true; // to activate/deactivate rendering of the skybox
    texture_info loadCubemap(const std::vector<std::string>& paths, int textureUnit = 0);
    texture_info skyboxTexture;
    glm::fmat4 skyboxModelMatrix{};
};

#endif