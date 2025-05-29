// We want the compiler to include this library one only once
#pragma once 

using namespace std;

#include <vector>

// Before the definition of the class we create the vertex struct to use to store all the vertex data
struct Vertex{
    // This struct is for the OpenGL data therefore we will rely on the GLM (mathematical library for OpenGL)
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

class Mesh{
    public:
        vector<Vertex> vertices; // Which vertices compose a triangle
        vector<GLuint> indices; 

        GLuint VAO; // I activate the number of the VAO containing the object I want to draw

        // We add the const to make sure that nothing gets modified when passed
        Mesh(const Mesh& copy) = delete;
        Mesh& operator=(const Mesh&) = delete;

        // I can initialize the vercies and indices by moving the memeory ownership. Also noexcept is to 
        Mesh(vector<Vertex>& vertices, vector<GLuint>& indices) noexcept : vertices(std::move(vertices)), indices(std::move(indices))
        {
            this->setupMesh();
        }

        // User defined move constructor
        Mesh(Mesh&& move) noexcept :  vertices(std::move(move.vertices)), indices(std::move(move.indices)), VAO(move.VAO), VBO(move.VBO), EBO(move.EBO)
        {
            // To avoid rendering by accident the object twice it is goo practice to set the previous VAO to 0 since the pointer would be still valid.
            move.VAO = 0;
        }

        Mesh& operator=(Mesh&& move) noexcept
        {
            freeGPUresources();

            // I check if the VAO is valid (the one that is incoming)
            if (move.VAO) 
            {
                vertices = std::move(move.vertices);
                indices = std::move(move.indices);
                VAO = move.VAO;
                VBO = move.VBO;
                EBO = move.EBO;

                // Once I moved the owenrship of the elements from move to the current one, I set the old one to 0;
                move.VAO = 0;
            }
            else // If I try to move the VAO that is already set to 0
            {
                VAO = 0;
            }

            return *this;
        }

        ~Mesh() noexcept
        {
            // Destruction method if the class is deleted or goes out of scope. It is called by default and we can user defn our own to accurately cleanup everything 
            freeGPUresources();
        }

        void Draw()
        {
            // Code taken from Work2. We render the model
            // Every time we want to draw an asset we tell it hich VAO to use
            glBindVertexArray(this->VAO);
            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0); // In this case the size will be given to us by the number of indices
            glBindVertexArray(0); // We are using a state machine therefore we need to remove the VAO from the state machine once we are done with it
        }

    private:
        // VBO and EBO can stay inside the private part since they don't need to be accessible from the outside 
        GLuint VBO, EBO;

        void setupMesh()
        {
            glGenVertexArrays(1, &this->VAO);
            glGenBuffers(1, &this->VBO);
            glGenBuffers(1, &this->EBO);
            
            // VAO is made "active"    
            glBindVertexArray(this->VAO);

            // we copy data in the VBO - we must set the data dimension, and the pointer to the structure cointaining the data
            glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
            glBufferData(GL_ARRAY_BUFFER, this->vertices.size()*sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW); //In this case the size is given by the number of verticex multiplied by the sizeof the struct Vertex
           
            // we copy data in the EBO - we must set the data dimension, and the pointer to the structure cointaining the data
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size()*sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW); //Size = to amount of indices * sizeof the data structure used to store them
        
            // we set in the VAO the pointers to the vertex positions (with the relative offsets inside the data structure)
            // these will be the positions to use in the layout qualifiers in the shaders ("layout (location = ...)"")    
            glEnableVertexAttribArray(0); // We are telling that of all the information about the vertex the first three ones 
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0); // We are telling to the VAO how the data in the VBO is organized. 

            // We need to give a new ID to the atribute, 0 is assigned for the vertexes, threfore 1 will be used for the normals
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex,Normal)); //I have to tell OpenGL from where it needs to start reading the data from the vertex
            // I tell OpenGL to start readig after an offset. it will read the next 3 elements after the offset

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex,TexCoords)); // I read 2 coordinates (U and V)

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex,Tangent));

            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex,Bitangent));
        
            glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
            glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO        
        }

        void freeGPUresources()
        {
            if(VAO)
            {
                // Properly de-allocate all resources once they've outlived their purpose
                glDeleteVertexArrays(1, &this->VAO);
                glDeleteBuffers(1, &this->VBO);
                glDeleteBuffers(1, &this->EBO);
            }
        }

};