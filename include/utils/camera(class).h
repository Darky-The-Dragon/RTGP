#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;

const GLfloat SPEED = 3.0f;
const GLfloat DIAGONAL_COMPENSATION = 0.70710678f;
const GLfloat SENSITIVITY = 0.25f;

class Camera
{
    public:
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 WorldFront;
        glm::vec3 Up;
        glm::vec3 WorldUp;
        glm::vec3 Right;
        GLboolean onGround;
        GLfloat Yaw;
        GLfloat Pitch;
        GLfloat MovementSpeed;
        GLfloat MovementCompensation = 1.0f;
        GLfloat MouseSensitivity;

        Camera(glm::vec3 position, GLboolean onGround) : Position(position), onGround(onGround), Yaw(YAW), Pitch(PITCH), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
        {
            this->WorldUp = glm::vec3(0.0f,1.0f,0.0f);
            this->UpdateCameraVectors();
        }

        glm::mat4 GetViewMatrix()
        {
            return glm::lookAt(this->Position, this->Position + this->Front, this->Up);
        }

        void SetMovementCompensation(GLboolean diagonal_movement)
        {
            this->MovementCompensation = (diagonal_movement ? DIAGONAL_COMPENSATION : 1.0f);
        }

        void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
        {
            GLfloat velocity = this->MovementSpeed * deltaTime * MovementCompensation;

            if(direction == FORWARD)
            {
                this->Position += (this->onGround ? this->WorldFront : this->Front) * velocity;
            }
            if(direction == BACKWARD)
            {
                this->Position -= (this->onGround ? this->WorldFront : this->Front) * velocity;
            }
            if(direction == LEFT)
            {
                this->Position -= this->Right * velocity;
            }
            if(direction == RIGHT)
            {
                this->Position += this->Right * velocity;
            }
        }

        void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE)
        {
            xoffset *= this->MouseSensitivity;
            yoffset *= this->MouseSensitivity;

            this->Yaw += xoffset;
            this->Pitch += yoffset;

            if(constraintPitch)
            {
                if(this->Pitch > 89.0f){
                    this->Pitch = 89.0f;
                }
                if(this->Pitch < -89.0f){
                    this->Pitch = -89.0f;
                }
            }
            this->UpdateCameraVectors();
        }

    private:
        void UpdateCameraVectors()
        {
            glm::vec3 front;
            front.x = cos(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
            front.y = sin(glm::radians(this->Pitch));
            front.z = sin(glm::radians(this->Yaw)) * cos(glm::radians(this->Pitch));
            this->Front = glm::normalize(front);
            front.y = 0.0;
            this->WorldFront = glm::normalize(front);
            this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
            this->Up = glm::normalize(glm::cross(this->Right, this->Front));
        }
};