// We want to develop a shader class that allows use to load whatever object we want
#pragma once 
// This tells the compiler to compile it only once. so if anything else includes this shader it won't be recompiled agian

using namespace std;

#include <string>
#include <fstream> // class to manage stream of data
#include <sstream> // class to manage stream of strings
#include <iostream> 

class Shader
{
    public:
        // The only attribute needed to be public is the ID so we can reference it
        GLuint Program; //unsigend int made by OpenGL specification. Normal uint should also work

        Shader(const GLchar* vertexPath, const GLchar* fragmentPath)
        {
            //we need o read and write the stream of data from the vertexPath and fragmentPath
            string vertexCode;
            string fragmentCode;
            ifstream vShaderFile;
            ifstream fShaderFile;

            // we need to check the ifstreams for exceptions to know if soemthing goes wrong. In this case we check for a fail in reading or a bad bit
            vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
            fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

            try 
            {
                // First we ope the 2 text files 
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                stringstream vShaderStream, fShaderStream;

                // We save the streams in a stringstream variable
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();

                // If no exception is launched then we successfully read the two files and can close the two streams;
                vShaderFile.close();
                fShaderFile.close();

                // We save the stream we read as a string in separate variables 
                vertexCode = vShaderStream.str();
                fragmentCode = fShaderStream.str();
            }
            catch(ifstream::failure const)
            {
                // If the shader fails we can stil continue with our program. The object will simply be black
                cout << "ERROR WHEN READING THE SHADER CODE" << endl;
            }

            // Since originally it was used with C instead of C++, strings weren't a thing. WE need to convert to pointers to chars
            const GLchar* vShaderCode = vertexCode.c_str();
            const GLchar* fShaderCode = fragmentCode.c_str();

            GLuint vertex, fragment;
            
            // Build and compile our shader program
            // Vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            // Here is where we will load the source 
            glShaderSource(vertex, 1, &vShaderCode, NULL); // name, 1 because we are passing one entity, where to find the source code, NULL because we pass the shader code as an external file
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX"); // we check for build errors
            
            // Fragment shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
            
            // Link shaders
            this->Program = glCreateProgram();
            glAttachShader(this->Program, vertex);
            glAttachShader(this->Program, fragment);
            glLinkProgram(this->Program);
            checkCompileErrors(this->Program, "PROGRAM");

            glDeleteShader(vertex);
            glDeleteShader(fragment);

        }

        void Use() {glUseProgram(this->Program);}

        void Delete() {glDeleteProgram(this->Program);}

    private:

        void checkCompileErrors(GLuint shader, string type)
        {
            // Check for compile time errors
            GLint success;
            GLchar infoLog[1024];

            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                // Checks if the ocmpialtion was successful
                if (!success)
                {
                    // It creates the shader program and links the two compiled objects in one program 
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << type << "\n" << std::endl;
                }
            }
            else 
            {
                glGetShaderiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    // It creates the shader program and links the two compiled objects in one program 
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER::LINKING_FAILED\n" << type << "\n" << std::endl;
                }
            }

        }
};
// Semicolon needs to be added at the end or it won't compile