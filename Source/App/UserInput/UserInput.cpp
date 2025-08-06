#include "UserInput.h"


namespace cve
{
    void KeyboardMovementController::MoveInPlaneXZ(GLFWwindow* window, float elapsedSec, GameObject& gameObject)
    {
        glm::vec2 rotate{ 0.0f };

        // --- Mouse capture and look-around ---
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS and !m_MouseCaptured) {
            m_MouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            m_PreviousMouseX = width * 0.5;
            m_PreviousMouseY = height * 0.5;
        }

        if (m_MouseCaptured) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            double dx = mouseX - m_PreviousMouseX;
            double dy = mouseY - m_PreviousMouseY;
            m_PreviousMouseX = mouseX;
            m_PreviousMouseY = mouseY;

            rotate.y += static_cast<float>(dx);
            rotate.x += static_cast<float>(-dy);
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS and m_MouseCaptured) {
            m_MouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }



        // 2) compute magnitude of the 2D mouse delta
        float deltaMag = glm::length(rotate);

        // 3) pick a scale in [m_LowSens…m_HighSens]
        float sensScale = (deltaMag < m_ThresholdPx)
            ? glm::mix(m_LowSens, m_HighSens, deltaMag / m_ThresholdPx)
            : m_HighSens;


        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
            // 4) apply scaled rotation
            glm::vec3 deltaRot = glm::vec3(rotate * sensScale, 0.0f);
            gameObject.m_Transform.rotation += m_LookSpeed * elapsedSec * deltaRot; 
        }

        // Clamp pitch (x) and wrap yaw (y)
        gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -1.5f, 1.5f);
        gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());

        // --- Floating camera axes (including pitch) ---
        float yaw = gameObject.m_Transform.rotation.y;
        float pitch = gameObject.m_Transform.rotation.x;

        // Compute raw direction based on camera orientation
        glm::vec3 dir{
            cos(pitch) * sin(yaw),
             -sin(pitch),
            cos(pitch) * cos(yaw)
        };
        // Use dir directly as forward, so pressing forward moves into view direction
        glm::vec3 forwardDir = glm::normalize(dir);
        glm::vec3 upDir = glm::vec3(0.0f, -1.0f, 0.0f);
        // Right vector is cross(up, forward)
        glm::vec3 rightDir = -glm::normalize(glm::cross(upDir, forwardDir));


        // Gather input
        glm::vec3 moveDir(0.0f);
        if (glfwGetKey(window, m_Keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, m_Keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, m_Keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, m_Keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, m_Keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, m_Keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        // Determine target speed (with sprint)
        float targetSpeed = m_MoveSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            targetSpeed *= m_SprintMultiplier;
        }

        // Compute and smooth velocity
        glm::vec3 targetVelocity(0.0f);
        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            targetVelocity = glm::normalize(moveDir) * targetSpeed;
        }
        glm::vec3 dv = targetVelocity - m_CurrentVelocity;
        float    maxStep = m_Acceleration * elapsedSec;
        float    len = glm::length(dv);
        if (len > maxStep) {
            m_CurrentVelocity += (dv / len) * maxStep;
        }
        else {
            m_CurrentVelocity = targetVelocity;
        }

        // Apply movement
        gameObject.m_Transform.translation += m_CurrentVelocity * elapsedSec;

    }

}
