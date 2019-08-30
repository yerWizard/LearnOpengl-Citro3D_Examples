#ifndef CAMERA_H
#define CAMERA_H

/*#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>*/
#include "citro3d.h"

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.04f;
const float ZOOM        =  45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Camera Attributes
    C3D_FVec Position;
    C3D_FVec Front;
    C3D_FVec Up;
    C3D_FVec Right;
    C3D_FVec WorldUp;
    // Euler Angles
    float Yaw;
    float Pitch;
    // Camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Constructor with vectors
    Camera(C3D_FVec position = FVec3_New(0.0f, 0.0f, 0.0f), C3D_FVec up = FVec3_New(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(FVec3_New(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(FVec3_New(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = FVec3_New(posX, posY, posZ);
        WorldUp = FVec3_New(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Returns the view matrix calculated using Euler Angles and the LookAt Matrix
    C3D_Mtx GetViewMatrix()
    {
        //return glm::lookAt(Position, Position + Front, Up);
        C3D_Mtx view;
        C3D_FVec posfront = FVec3_Add(Position, Front);
        Mtx_LookAt(&view, Position, posfront, Up, false);

        return view;
    }

    // Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        C3D_FVec vect;
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD){
            vect = FVec3_Scale(Front, velocity);
            Position = FVec3_Add(Position,vect);
        }
        if (direction == BACKWARD){
            vect = FVec3_Scale(Front, velocity);
            Position =  FVec3_Subtract(Position, vect);
        }

        if (direction == LEFT){
            vect = FVec3_Scale(Right, velocity);
            Position = FVec3_Subtract(Position, vect);
        }
        if (direction == RIGHT){
            vect = FVec3_Scale(Right, velocity);
            Position = FVec3_Add(Position, vect);
        }
    }

    // Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        if (Zoom >= 1.0f && Zoom <= 45.0f)
            Zoom -= yoffset;
        if (Zoom <= 1.0f)
            Zoom = 1.0f;
        if (Zoom >= 45.0f)
            Zoom = 45.0f;
    }

private:
    // Calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // Calculate the new Front vector
        C3D_FVec front;
        front.x = cos(C3D_AngleFromDegrees(Yaw)) * cos(C3D_AngleFromDegrees(Pitch));
        front.y = sin(C3D_AngleFromDegrees(Pitch));
        front.z = sin(C3D_AngleFromDegrees(Yaw)) * cos(C3D_AngleFromDegrees(Pitch));
        Front = FVec3_Normalize(front);
        // Also re-calculate the Right and Up vector
        Right = FVec3_Normalize(FVec3_Cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up    = FVec3_Normalize(FVec3_Cross(Right, Front));
    }
};
#endif