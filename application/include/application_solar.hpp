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
#include "planet.hpp"

// added for assignment 2
#include <cstdlib> // for random
#include <time.h> // for random seed
#include <vector>

// added for assignment 4
#include "texture_info.hpp"

// added for additional tasks
#include "text_loader.hpp"


// gpu representation of model
class ApplicationSolar : public Application
{
  public:

    // allocate and initialize objects
    ApplicationSolar(std::string const& resource_path, const unsigned windowWidth, const unsigned windowHeight);

    // free allocated objects
    ~ApplicationSolar();

    // update uniform locations and values
    void uploadUniforms();

    // uploads texture uniforms
    void uploadTexture(GLuint sampler_location, texture_info texinf) const;

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
    
    // render all the texts
    void renderText() const;

    // render 3D text of planets
    void renderPlanetTexts(std::shared_ptr<Planet> planet, glm::fvec3& camPos) const;

    // uses this shader and remembers the last one
    // returns true if the shader state changed
    bool useShader(std::string shaderName) const;

    // use the framebuffer to draw
    void frameBufferDrawing() const;

    // to enable/disable specific effects
    void toggleEffect(unsigned char& effectFlags, int effectFlag, std::string name);

    // adds 3D text for the planet (set moons to true to add text for its moons too)
    void add3DText(std::shared_ptr<Planet> planet, bool moons);


  protected:

    void initializeShaderPrograms();
    void initializeGeometry();
    void initializePlanets();
    void initializeStars();
    void initializeSkybox();
    bool initializeFrameBuffer();
    bool initializeFonts();
    void initializeTexts(TextLoader& tl);
    void updateView();
    void move();

    // uniform buffers (assignment 6)
    bool initializeUniformBuffers();
    void updateUniformBuffer(GLuint bufferHandle, void* sourceData, size_t sourceDataSize);
    void queryUniformBlockIndex(GLuint blockIndex, std::string shaderName, char* uniformName);

    // cpu representation of model
    model_object planet_object;
    model_object stars{};
    model_object skybox_object;
    model_object screenQuad; // the quad to draw the framebuffer


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
    float cameraRotationSpeed = 2.f;

    // store information about the camera rotation around the x-axis
    float cameraRotationX = 0.f;
    float cameraRotationX_max = MATH_HALF_PI;
    float cameraRotationX_min = -MATH_HALF_PI;

    // how many times faster the camera moves on sprinting
    float cameraSprintMultiplier = 2.0f;

    // store information about how the movement should be done
    glm::fvec3 movementVector{0.f, 2.f, 30.f}; // initial camera position

    unsigned int starCount = 2500;
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
    bool useNormalMapping = true; // to enable/disable normal mapping
    bool showOrbits = true; // to enable/disable orbits

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

    // Framebuffer variables
    bool useFrameBuffer = true; // enable/disable use of framebuffer (before starting the program!)
    GLuint frameBuffer;
    GLuint frameBufferTexture;
    GLuint depthRenderBuffer;
    unsigned char screenEffects = 0; // tells the shader which effects to apply
    glm::fmat3 blurKernel = glm::fmat3{1.f/16.f, 2.f/16.f, 1.f/16.f,
                                       2.f/16.f, 4.f/16.f, 2.f/16.f,
                                       1.f/16.f, 2.f/16.f, 1.f/16.f};

    // struct for uniform buffer object
    CameraBuffer camBuffer{};
    GLuint gpu_minBindingPoints = 1; // amount of binding points needed
    GLuint UBO_camera;        // uniform buffer object handle for camera data
    GLuint UBO_camera_bi = 0; // global binding index of the UBO

    bool renderTexts = true;
    std::shared_ptr<TextLoader> textLoader;
    mutable std::vector<std::shared_ptr<Text2D>> texts2D;
    mutable std::vector<std::shared_ptr<Text3D>> texts3D; // stored in planet object as well
    std::vector<std::shared_ptr<Text>> texts;
};


// effect flags (have to be the same as in the shader)
#define FLAG_EFFECT_GRAYSCALE           0x1
#define FLAG_EFFECT_MIRRORED_HORIZONTAL 0x2
#define FLAG_EFFECT_MIRRORED_VERTICAL   0x4
#define FLAG_EFFECT_BLURRED             0x8


#endif