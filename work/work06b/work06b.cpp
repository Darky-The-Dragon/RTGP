/*
work06b

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano
*/



// Std. Includes
#include <string>

// Loader estensioni OpenGL
// http://glad.dav1d.de/
// THIS IS OPTIONAL AND NOT REQUIRED, ONLY USE THIS IF YOU DON'T WANT GLAD TO INCLUDE windows.h
// GLAD will include windows.h for APIENTRY if it was not previously defined.
// Make sure you have the correct definition for APIENTRY for platforms which define _WIN32 but don't use __stdcall
#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders, to load models, for FPS camera, and for physical simulation
#include <utils/shader.h>
#include <utils/model.h>
#include <utils/camera.h>
#include <utils/physics(class).h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

GLuint screenWidth = 1200, screenHeight = 900;

// callback functions for keyboard and mouse events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// if one of the WASD keys is pressed, we call the corresponding method of the Camera class
void apply_camera_movements();

// we initialize an array of booleans for each keyboard key
bool keys[1024];

// we set the initial position of mouse cursor in the application window
GLfloat lastX = 400.0f, lastY = 300.0f;

// we will use these value to "pass" the cursor position to the keyboard callback, in order to determine the bullet trajectory
double cursorX,cursorY;

// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat currentFrame = 0.0;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// view and projection matrices (global because we need to use them in the keyboard callback)
glm::mat4 view, projection;

// we create a camera. We pass the initial position as a parameter to the constructor. In this case, we use a "floating" camera (we pass false as last parameter)
Camera camera(glm::vec3(0.0f, 0.0f, 9.0f), GL_FALSE);

// Uniforms to be passed to shaders
// point light position
glm::vec3 lightPos0 = glm::vec3(5.0f, 10.0f, 10.0f);

// weight for the diffusive component
GLfloat Kd = 3.0f;
// roughness index for GGX shader
GLfloat alpha = 0.2f;
// Fresnel reflectance at 0 degree (Schlik's approximation)
GLfloat F0 = 0.9f;

// color of the falling objects
GLfloat diffuseColor[] = {1.0f,0.0f,0.0f};
// color of the plane
GLfloat planeMaterial[] = {0.0f,0.5f,0.0f};
// color of the bullets
GLfloat shootColor[] = {1.0f,1.0f,0.0f};
glm::vec3 sphere_size = glm::vec3(0.2f, 0.2f, 0.2f);
GLfloat shootInitialSpeed = 15.0f;

Physics bulletSimulation;

// variables used to store uniform location inside shaders
GLint objDiffuseLocation, pointLightLocation, kdLocation, alphaLocation, f0Location;


////////////////// MAIN function ///////////////////////
int main()
{
  // Initialization of OpenGL context using GLFW
  glfwInit();
  // We set OpenGL specifications required for this application
  // In this case: 4.1 Core
  // If not supported by your graphics HW, the context will not be created and the application will close
  // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
  // in relation also to the values indicated in these GLFW commands
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // we set if the window is resizable
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RTGP_work06b", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    //the "clear" color for the frame buffer
    glClearColor(0.26f, 0.46f, 0.98f, 1.0f);

    // the Shader Program for the objects used in the application
    Shader object_shader = Shader("09_illumination_models.vert", "10_illumination_models.frag");

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");

    // dimensions and position of the static plane
    // we will use the cube mesh to simulate the plane, because we need some "height" in the mesh
    // in order to make it work with the physics simulation
    glm::vec3 plane_pos = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 plane_size = glm::vec3(200.0f, 0.1f, 200.0f);
    glm::vec3 plane_rot = glm::vec3(0.0f, 0.0f, 0.0f);

    btRigidBody* plane = bulletSimulation.createRigidBody(BOX, plane_pos, plane_size, plane_rot, 0.0f, 0.3f, 0.3f);

    // we create 25 rigid bodies for the cubes of the scene. In this case, we use BoxShape, with the same dimensions of the cubes, as collision shape of Bullet. For more complex cases, a Bounding Box of the model may have to be calculated, and its dimensions to be passed to the physics library
    GLint num_side = 5;
    // total number of the cubes
    GLint total_cubes = num_side*num_side;
    GLint i,j;
    // position of the cube
    glm::vec3 cube_pos;
    // dimension of the cube
    glm::vec3 cube_size = glm::vec3(0.2f, 0.5f, 0.2f);
    // we set a small initial rotation for the cubes
    glm::vec3 cube_rot = glm::vec3(0.1f, 0.0f, 0.1f);

    //vector<glm::vec3> positions;

    btRigidBody* cube;

    // we create a 5x5 grid of rigid bodies
    for(i = 0; i < num_side; i++ )
    {
        for(j = 0; j < num_side; j++ )
        {
            cube_pos = glm::vec3((i - num_side)+3.0f, 1.0f, (num_side - j));
            cube = bulletSimulation.createRigidBody(BOX, cube_pos, cube_size, cube_rot, 2.0f, 0.3f, 0.3f);
        }
    }

    // Max deltaTime of the physic simulation
    GLfloat maxSecPerFrame = 1.0f / 60.0f;

  // Projection matrix: FOV angle, aspect ratio, near and far planes
  projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

  // Model and Normal transformation matrices for the objects in the scene: we set to identity
  glm::mat4 objModelMatrix = glm::mat4(1.0f);
  glm::mat3 objNormalMatrix = glm::mat3(1.0f);
  glm::mat4 planeModelMatrix = glm::mat4(1.0f);
  glm::mat3 planeNormalMatrix = glm::mat3(1.0f);

  // Rendering loop: this code is executed at each frame
  while(!glfwWindowShouldClose(window))
  {
      // we determine the time passed from the beginning
      // and we calculate time difference between current frame rendering and the previous one
      currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // Check is an I/O event is happening
      glfwPollEvents();
      // we apply FPS camera movements
      apply_camera_movements();
      // View matrix (=camera): position, view direction, camera "up" vector
      // in this example, it has been defined as a global variable (we need it in the keyboard callback function)
      view = camera.GetViewMatrix();

      // we "clear" the frame and z buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      // we set the rendering mode
      if (wireframe)
          // Draw in wireframe
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        bulletSimulation.dynamicsWorld->stepSimulation((deltaTime<maxSecPerFrame ? deltaTime : maxSecPerFrame), 10);

      /////////////////// OBJECTS ////////////////////////////////////////////////
      // We "install" the selected Shader Program as part of the current rendering process
      object_shader.Use();
      // We search inside the Shader Program the name of a subroutine, and we get the numerical index
      GLuint index = glGetSubroutineIndex(object_shader.Program, GL_FRAGMENT_SHADER, "GGX");
      // we activate the subroutine using the index
      glUniformSubroutinesuiv( GL_FRAGMENT_SHADER, 1, &index);

      // we pass projection and view matrices to the Shader Program
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

      // we determine the position in the Shader Program of the uniform variables
      objDiffuseLocation = glGetUniformLocation(object_shader.Program, "diffuseColor");
      pointLightLocation = glGetUniformLocation(object_shader.Program, "pointLightPosition");
      kdLocation = glGetUniformLocation(object_shader.Program, "Kd");
      alphaLocation = glGetUniformLocation(object_shader.Program, "alpha");
      f0Location = glGetUniformLocation(object_shader.Program, "F0");

      // we assign the value to the uniform variable
      glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos0));
      glUniform1f(kdLocation, Kd);
      glUniform1f(alphaLocation, alpha);
      glUniform1f(f0Location, F0);

      /////
      // STATIC PLANE
      // we use a specific color for the plane
      glUniform3fv(objDiffuseLocation, 1, planeMaterial);

      planeModelMatrix = glm::mat4(1.0f);
      planeNormalMatrix = glm::mat3(1.0f);
      planeModelMatrix = glm::translate(planeModelMatrix, plane_pos);
      planeModelMatrix = glm::scale(planeModelMatrix, plane_size);
      planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
      glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
      glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));

      // we render the plane
      cubeModel.Draw();
      planeModelMatrix = glm::mat4(1.0f);

      /////
      // DYNAMIC OBJECTS (FALLING CUBES + BULLETS)
      /////

      glUniform3fv(objDiffuseLocation, 1, diffuseColor);

      GLfloat matrix[16];
      btTransform transform;

      glm::vec3 obj_size;
      Model* objectModel;

      int num_cobjs = bulletSimulation.dynamicsWorld->getNumCollisionObjects();

      for(i = 1; i < num_cobjs; i++)
      {
        if(i <= total_cubes)
        {
            objectModel = &cubeModel;
            obj_size = cube_size;
            glUniform3fv(objDiffuseLocation, 1, diffuseColor);
        }
        else
        {
            objectModel = &sphereModel;
            obj_size = sphere_size;
            glUniform3fv(objDiffuseLocation, 1, shootColor);
        }
        btCollisionObject* obj = bulletSimulation.dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        body->getMotionState()->getWorldTransform(transform);
        transform.getOpenGLMatrix(matrix);

        objModelMatrix = glm::mat4(1.0f);
        objNormalMatrix = glm::mat3(1.0f);

        objModelMatrix = glm::make_mat4(matrix) * glm::scale(objModelMatrix, obj_size);

        objNormalMatrix = glm::inverseTranspose(glm::mat3(view*objModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(object_shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(objModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(object_shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(objNormalMatrix));

        // renderizza il modello
        objectModel->Draw();
        objModelMatrix = glm::mat4(1.0f);
    }

      // Faccio lo swap tra back e front buffer
      glfwSwapBuffers(window);
  }

  // when I exit from the graphics loop, it is because the application is closing
  // we delete the Shader Programs
  object_shader.Delete();
  bulletSimulation.Clear();

  // we close and delete the created context
  glfwTerminate();
  return 0;
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
    // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
    // then the camera will apply a compensation factor to the velocities applied in the single directions, 
    // in order to have the full v applied in the diagonal direction  
    // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.  
    GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
    camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)

    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // we save the current cursor position in 2 global variables, in order to use the values in the keyboard callback function
    cursorX = xpos;
    cursorY = ypos;

    // offset of mouse cursor position
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;

    // the new position will be the previous one for the next frame
    lastX = xpos;
    lastY = ypos;

    // we pass the offset to the Camera class instance in order to update the rendering
    camera.ProcessMouseMovement(xoffset, yoffset);

}
