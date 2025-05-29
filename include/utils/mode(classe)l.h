#pragma once

using namespace std;

#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <utils/mesh.h>

class Model
{
    public:
        vector<Mesh> meshes;

        Model(const Model& copy) = delete;
        Model& operator=(const Model&) = delete;

        // We are telling the compile to use the standart move operators
        Model(Model&& move) = default;
        Model& operator=(Model&&) noexcept = default;

        // Constructor of the model class:
        // It will be a string with the path to the model
        Model(const string& path)
        {
            // Method that opens and reads the file
            this->loadModel(path);
        }

        // The draw method of the Model simply calls the Draw method of Mesh to rneder stuff on openGL
        void Draw()
        {
            for(GLuint i = 0; i < this->meshes.size(); i++)
            {
                this->meshes[i].Draw();
            }
        }

    private:
        void loadModel(string path)
        {
            Assimp::Importer importer;
            // The importer has a method called "ReadFile" to which i give the path plus a sequence of flags. In return we get a pointer to aiScene (assmip importer scene)
            // With aiProcess_Triangulate I tell assimp to traignulate (in case the mesh is in quads). I coult in theory develop my own code if I hate myself
            // With aiProcess_JoinIdenticalVertices I tell assimp to join vertices that are in the same position
            // With aiProcess_FlipUVs I tell assimp to flip UVs for OpenGL when needed

            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

            // I check if scene is not empty (aka wrong path), if scene got a corrupted file or if scene is valid but has no root inside from where to start.
            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                cout << "ERROR LOAD SCENE " << importer.GetErrorString() << endl;
                return;
            }

            // If I am here meas I read the file without any issues;
            this->processNode(scene->mRootNode, scene);
        }

        void processNode(aiNode* node, const aiScene* scene)
        {
            // It's a tree data structure therefore every node mght have multiple meshes inside
            for(GLuint i = 0; i<node->mNumMeshes; i++)
            {
                aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
                this->meshes.emplace_back(processMesh(mesh));
            }

            for(GLuint i = 0; i<node->mNumChildren; i++)
            {
                // I call recursively this method to pass through all the nodes
                this->processNode(node->mChildren[i], scene);
            }
        }

        Mesh processMesh(aiMesh* mesh)
        {
            vector<Vertex> vertices;
            vector<GLuint> indices;

            for(GLuint i = 0; i < mesh->mNumVertices; i++){
                Vertex vertex;
                glm::vec3 vector;

                vector.x = mesh->mVertices[i].x;
                vector.y = mesh->mVertices[i].y;
                vector.z = mesh->mVertices[i].z;
                vertex.Position = vector;

                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;

                if (mesh->mTextureCoords[0])
                {
                    glm::vec2 vec;

                    vec.x = mesh->mTextureCoords[0][i].x;
                    vec.y = mesh->mTextureCoords[0][i].y;
                    vertex.TexCoords = vec;

                    vector.x = mesh->mTangents[i].x;
                    vector.y = mesh->mTangents[i].y;
                    vector.z = mesh->mTangents[i].z;
                    vertex.Tangent = vector;

                    vector.x = mesh->mBitangents[i].x;
                    vector.y = mesh->mBitangents[i].y;
                    vector.z = mesh->mBitangents[i].z;
                    vertex.Bitangent = vector;
                }
                else
                {
                    // If it fails we can have a dummy value inside to let the program work
                    vertex.TexCoords = glm::vec2(0.0f, 0.0f);
                    cout << "NO UV FOUND" << endl;
                }

                vertices.emplace_back(vertex);
            }

            for (GLuint i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace* face = &mesh->mFaces[i];
                for (GLuint j = 0; j < face->mNumIndices; j++)
                {
                    indices.emplace_back(face->mIndices[j]);
                }
            }
            
            return Mesh(vertices, indices);
        }

        // Good practice would be to write the destructr but since it's a very simple one we can use the default implicit one.
};
