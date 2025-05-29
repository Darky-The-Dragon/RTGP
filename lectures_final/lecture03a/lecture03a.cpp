/*
Es03a: Basic shaders, animation of rotation transform, wireframe visualization
- swapping between different basic shaders pressing keys from 1 to 5

N.B. 1)
The lecture uses a very "basic" shader swapping method.
This method was used until OpenGL 3.3 (thus, still valid for OpenGL ES and WebGL), and it can still be used for simple scenes and few shaders.
In the next lectures, a more advances method based on Shader Subroutines will be used.

N.B. 2) no texturing in this version of the classes

N.B. 3) to test different parameters of the shaders, it is convenient to use some GUI library, like e.g. Dear ImGui (https://github.com/ocornut/imgui)

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

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// setup of Shader Programs for the 5 shaders used in the application
void SetupShaders();
// delete Shader Programs whan application ends
void DeleteShaders();
// print on console the name of current shader
void PrintCurrentShader(int shader);

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat currentFrame = 0.0; 

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{ FULLCOLOR, FLATTEN, NORMAL2COLOR, WAVE, UV2COLOR };
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "FULLCOLOR", "FLATTEN", "NORMAL2COLOR", "WAVE", "UV2COLOR" };

// index of the current shader (= 0 in the beginning)
GLuint current_program = FULLCOLOR;
// a vector for all the Shader Programs used and swapped in the application
vector<Shader> shaders;

// Uniforms to be passed to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
// weight and velocity for the animation of Wave shader
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;
// variables used to store uniform location inside shaders
GLint fragColorLocation, weightLocation, timerLocation;


/////////////////// MAIN function ///////////////////////
int main()
{
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    // It is possible to raise the values, in order to use functionalities of more recent OpenGL specs.
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
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RGP_lecture03a", nullptr, nullptr);
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

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create the Shader Programs used in the application
    SetupShaders();

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model cubeModel("../../models/cube.obj");
    Model sphereModel("../../models/sphere.obj");
    Model bunnyModel("../../models/bunny_lp.obj");

    // we print on console the name of the first shader used
    PrintCurrentShader(current_program);

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 7.0f), glm::vec3(0.0f, 0.0f, -7.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);
    glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
    glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);

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

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, then we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        // We "install" the selected Shader Program as part of the current rendering process
        shaders[current_program].Use();

        // uniforms are passed to the corresponding shader
        if (current_program == FULLCOLOR || current_program == FLATTEN)
        {
            // we determine the position in the Shader Program of the uniform variable
            fragColorLocation = glGetUniformLocation(shaders[current_program].Program, "colorIn");
            // we assign the value to the uniform variable
            glUniform3fv(fragColorLocation, 1, myColor);
        }
        else if (current_program == WAVE)
        {

            // we determine the position in the Shader Program of the uniform variables
            weightLocation = glGetUniformLocation(shaders[current_program].Program, "weight");
            timerLocation = glGetUniformLocation(shaders[current_program].Program, "timer");
            // we assign the value to the uniform variables
            glUniform1f(weightLocation, weight);
            glUniform1f(timerLocation, currentFrame*speed);
        }

        // we pass projection and view matrices to the Shader Program
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));

        //SPHERE
        /*
          we create the transformation matrix
          N.B.) the last defined is the first applied

          We need also the matrix for normals transformation, which is the inverse of the transpose of the 3x3 submatrix (upper left) of the modelview.
          We do not consider the 4th column because we do not need translations for normals.
          An explanation (where XT means the transpose of X, etc):
            "Two column vectors X and Y are perpendicular if and only if XT.Y=0.
            If we're going to transform X by a matrix M, we need to transform Y by some matrix N so that (M.X)T.(N.Y)=0.
            Using the identity (A.B)T=BT.AT, this becomes (XT.MT).(N.Y)=0 => XT.(MT.N).Y=0.
            If MT.N is the identity matrix then this reduces to XT.Y=0.
            And MT.N is the identity matrix if and only if N=(MT)-1, i.e. N is the inverse of the transpose of M.

        */
        // we reset to identity at each frame
        sphereModelMatrix = glm::mat4(1.0f);
        sphereNormalMatrix = glm::mat3(1.0f);
        sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(-3.0f, 0.0f, 0.0f));
        sphereModelMatrix = glm::rotate(sphereModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));	// It's a bit too big for our scene, so scale it down
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view*sphereModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix));

        // we render the sphere
        sphereModel.Draw();

        //CUBE
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        cubeModelMatrix = glm::mat4(1.0f);
        cubeNormalMatrix = glm::mat3(1.0f);
        cubeModelMatrix = glm::translate(cubeModelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        cubeModelMatrix = glm::rotate(cubeModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeModelMatrix = glm::scale(cubeModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));	// It's a bit too big for our scene, so scale it down
        cubeNormalMatrix = glm::inverseTranspose(glm::mat3(view*cubeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cubeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(cubeNormalMatrix));

        // we render the cube
        cubeModel.Draw();

        //BUNNY
        // we create the transformation matrix and the normals transformation matrix
        // we reset to identity at each frame
        bunnyModelMatrix = glm::mat4(1.0f);
        bunnyNormalMatrix = glm::mat3(1.0f);
        bunnyModelMatrix = glm::translate(bunnyModelMatrix, glm::vec3(3.0f, 0.0f, 0.0f));
        bunnyModelMatrix = glm::rotate(bunnyModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        bunnyModelMatrix = glm::scale(bunnyModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));	// It's a bit too big for our scene, so scale it down
        bunnyNormalMatrix = glm::inverseTranspose(glm::mat3(view*bunnyModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaders[current_program].Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(shaders[current_program].Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(bunnyNormalMatrix));

        // we render the bunny
        bunnyModel.Draw();

        // Swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShaders();
    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// we create and compile shaders (code of Shader class is in include/utils/shader.h), and we add them to the list of available shaders
void SetupShaders()
{
    Shader shader1("00_basic.vert", "01_fullcolor.frag");
    shaders.push_back(shader1);
    Shader shader2("02_flatten.vert", "02_flatten.frag");
    shaders.push_back(shader2);
    Shader shader3("03_normal2color.vert", "03_normal2color.frag");
    shaders.push_back(shader3);
    Shader shader4("04_wave.vert", "04_wave.frag");
    shaders.push_back(shader4);
    Shader shader5("05_uv2color.vert", "05_uv2color.frag");
    shaders.push_back(shader5);
}

//////////////////////////////////////////
// we delete all the Shaders Programs
void DeleteShaders()
{
    for(GLuint i = 0; i < shaders.size(); i++)
        shaders[i].Delete();
}

//////////////////////////////////////////
// we print on console the name of the currently used shader
void PrintCurrentShader(int shader)
{
    std::cout << "Current shader:" << print_available_ShaderPrograms[shader]  << std::endl;

}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // pressing a key between 1 and 5, we change the shader applied to the models
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_5) && action == GLFW_PRESS)
    {
        // "1" to "5" -> ASCII codes from 49 to 57
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
        // we subtract 1 to have indices from 0 to 4 in the shaders list
        current_program = (key-'0'-1);
        PrintCurrentShader(current_program);
    }
}