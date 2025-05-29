/*
work03

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2024/2025
Master degree in Computer Science
Universita' degli Studi di Milano
*/

/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

// Std. Includes
#include <string>

// Loader for OpenGL extensions
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

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader.h>
#include <utils/model.h>
// We need to use th elibraries to set up matrixes -> GLM library 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Since we want to have an animation / we want to use animated shaders, we will need variables for the time management
GLfloat deltaTime = 0.0f; // Keeps track of the time
GLfloat lastFrame = 0.0f; // Keeps track of the time that pases between the current frame and the previous frame

GLfloat orientationY = 0.0f; // Variable that keeps track of the angle around the axis that changes frmae by frame in relation to the time 
GLfloat spin_speed = 30.0f;
// Booleans to keep track if the object is spinning or showing it's wireframe
GLboolean spinning = GL_TRUE;
GLboolean wireframe = GL_FALSE;

// GLfloat myColor[] = {0.0f, 1.0f, 0.0f};
// GLfloat weight = 0.2f;
// GLfloat speed = 5.0f;
GLfloat frequency = 10.0f;
GLfloat power = 1.0f;
GLfloat harmonics = 4.0f;

/////////////////// MAIN function ///////////////////////
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RTGP_work03", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    // the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create and compile shaders (code of Shader class is in include/utils/shader.h)
    Shader shader("00_basic.vert", "00_basic.frag");

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");
    Model bunnyModel("../../models/bunny_lp.obj");

    // We "install" the Shader Program as part of the current rendering process
    // with only one Shader Program, we can do this call now, and not inside the main loop:
    // we will use this Shader Program for everything is rendered after this call
    shader.Use();   // <-- Don't forget this one!

    // We set up now the transformation. In this example the camera will be fixed while looking down at the scene.
    // We need the view matrix and projection matrix. We can set up the matrixes before the rendering loop since they are fixed. 
    // Next week we will move the view matrix into the rendering loop since we will impleent a moving camera

    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f); // 4 parameters: FOV, aspect ration, near and far for the view frustum
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // It requires 3 vec3 parameteres

    // For every object we need the model matrix for the transforamtions anda matrix for the normal transformations
    glm::mat4 cubeModelMatrix = glm::mat4(1.0f); // By typing this I don't get a matrix full of 1. I create this way an Identity Matrix
    glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f); 
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f); 
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);

    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // We first detect the time for the animation of our models. We want the precise time as soon as the rendering loop starts. This time management isn't done by OpenGL but by the library managing
        // the window that displays the environment.
        GLfloat currentFrame = glfwGetTime(); // Geral time since the start of the application
        deltaTime = currentFrame - lastFrame; // Delta between global time - the global time of the previous frame that got rendered
        lastFrame = currentFrame;

        // Check is an I/O event is happening
        glfwPollEvents();

        // we "clear" the frame and z  buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // We set up the viualisation: full color or wireframe. We need to set it up before calling the draw method.
        if(wireframe){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // We tell it to draw the front and back faces with just lines
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // GL_FRONT_AND_BACK renders front and back faces
        }

        if(spinning){
            orientationY += (deltaTime*spin_speed); // Always use an increment. Never use a state otherise we wiill notice sudden changes in the movement whenever we pause and resume the animation
        }

        // GLint fragColorLocation = glGetUniformLocation(shader.Program, "colorIn");
        // glUniform3fv(fragColorLocation, 1, myColor); // Here we don't need a pointer by using value_ptr.

        // GLint weightLocation = glGetUniformLocation(shader.Program, "weight");
        // glUniform1f(weightLocation, weight);

        // GLint timerLocation = glGetUniformLocation(shader.Program, "timer");
        // glUniform1f(timerLocation, currentFrame * speed);

        GLint frequencyLocation = glGetUniformLocation(shader.Program, "frequency");
        glUniform1f(frequencyLocation, frequency); // Here we don't need a pointer by using value_ptr.

        GLint powerLocation = glGetUniformLocation(shader.Program, "power");
        glUniform1f(powerLocation, power);

        GLint harmonicsLocation = glGetUniformLocation(shader.Program, "harmonics");
        glUniform1f(harmonicsLocation, harmonics);

        GLint timerLocation = glGetUniformLocation(shader.Program, "timer");
        glUniform1f(timerLocation, currentFrame);

        // It required the ID of the shader program to search for the variable. In this case shader.Program. 
        // Then it needs the name of the variable inside the shader. We need to be careful when writing the name of the variable -> otherwise we will have a silent error. The program will display simply nothing
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        // Now we need to setup the model transformations of the cube before drawing it. We need to setup the matrix of the scale, rotation and traslation:
        cubeModelMatrix = glm::mat4(1.0f);
        cubeNormalMatrix = glm::mat3(1.0f);

        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        cubeModelMatrix = glm::rotate(cubeModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        cubeNormalMatrix = glm::inverseTranspose(glm::mat3(view*cubeModelMatrix)); // Automatically does a downcast from a 4x4 matrix to a 3x3 matrix 
        
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cubeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cubeNormalMatrix));

        // We render the cube model
        cubeModel.Draw();

        // Now we need to setup the model transformations of the sphere before drawing it. We need to setup the matrix of the scale, rotation and traslation:
        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);

        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 0.0f, 0.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix)); // Automatically does a downcast from a 4x4 matrix to a 3x3 matrix 
        
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

        // we render the sphere model
        sphereModel.Draw();

        // Now we need to setup the model transformations of the cube before drawing it. We need to setup the matrix of the scale, rotation and traslation:
        bunnyModelMatrix = glm::mat4(1.0f);
        bunnyNormalMatrix = glm::mat3(1.0f);

        bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));
        bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.33f, 0.3f));
        bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view*bunnyModelMatrix)); // Automatically does a downcast from a 4x4 matrix to a 3x3 matrix 
        
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyNormalMatrix));

        // we render the bunny model
        bunnyModel.Draw();

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Program
    shader.Delete();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning = !spinning;

    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe = !wireframe;
}
