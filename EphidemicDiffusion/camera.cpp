#include "camera.h"

//#include <glm/gtc/quaternion.hpp>

camera::camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_front(glm::vec3(0.0f, 0.0f, -1.0f)), m_movementSpeed(SPEED), m_mouseSensitivity(SENSITIVITY)
{
    m_position = position;
    m_worldUp = up;
    m_yaw = yaw;
    m_pitch = pitch;
    updateCameraVectors();
}

void camera::processKeyboard(Camera_Movement direction, float deltaTime) {

    float velocity = m_movementSpeed * deltaTime;
    if (direction == Camera_Movement::FORWARD) {
        m_position += m_front * velocity;
    }
    if (direction == Camera_Movement::BACKWARD) {
        m_position -= m_front * velocity;
    }
    if (direction == Camera_Movement::LEFT) {
        m_position -= m_right * velocity;
    }
    if (direction == Camera_Movement::RIGHT) {
        m_position += m_right * velocity;
    }
}

glm::mat4 camera::getViewMatrix() {
    return glm::lookAt(m_position, m_position + m_front, m_up);
}

void camera::processMouseMovement(float xOffset, float yOffset, GLboolean constrainPitch) {
    xOffset *= m_mouseSensitivity;
    yOffset *= m_mouseSensitivity;

    m_yaw += xOffset;
    m_pitch += yOffset;

    if (constrainPitch) {
        if (m_yaw > 89.0f)
            m_yaw = 89.0f;
        else if (m_yaw < -89.0f)
            m_yaw = -89.0f;
    }

    updateCameraVectors();
}

void camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));
    front.y = glm::sin(glm::radians(m_pitch));
    front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch));

    m_front = glm::normalize(front);
    m_right = glm::normalize(glm::cross(m_front, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_front));

}
