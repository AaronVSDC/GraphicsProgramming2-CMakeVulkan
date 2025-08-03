#pragma once


#include "GameObject.h"
#include "Window.h"


namespace cve
{
    class KeyboardMovementController
    {

    public:
        struct KeyMappings
        {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_CONTROL;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };


        void MoveInPlaneXZ(GLFWwindow* window, float dt, GameObject& gameObject);


    private:

        // Mouse capture state
        bool m_MouseCaptured = false;
        double m_PreviousMouseX = 0.0, m_PreviousMouseY = 0.0;

        // Movement smoothing and speed settings
        glm::vec3 m_CurrentVelocity = glm::vec3(0.0f);
        float m_MoveSpeed = 500.0f;  // base move speed (units/sec)
        float m_LookSpeed = .5f;  // camera look sensitivity
        float m_SprintMultiplier = 2.0f;   // sprint speed multiplier
        float m_Acceleration = 1000.f;  // acceleration/deceleration (units/sec^2)

        // Mouse sensitivity smoothing
        float    m_LowSens = 0.2f;    // sensitivity factor when very slight
        float    m_HighSens = 1.0f;    // full sensitivity beyond threshold
        float    m_ThresholdPx = 2.0f;    // how many pixels before we hit full sens


        KeyMappings m_Keys;



    };



}
