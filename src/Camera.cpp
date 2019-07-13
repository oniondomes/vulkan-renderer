#include "Camera.hpp"
#include "Input.hpp"

float yaw = 90.0f;
float pitch = 0.0f;

Camera::Camera(const float ratio)
{
    _fov = glm::radians(65.0f);
    _ratio = 1.0f;

    _near = 0.01f;
    _far = 100.0f;

    _movementSpeed = 0.05f;
    _rotationSpeed = 0.1f;

    _position = glm::vec3(2.0f, 2.0f, 0.0f);
    _up = glm::vec3(0.0f, 1.0f, 0.0f);
    _front = glm::vec3(0.0f, 0.0f, 1.0f);

    _updateProjection();
    _updateView();

    glm::vec3 direction = _front;
}

void Camera::update()
{
    _updateView();

    updateWithKeys();

    if (Input::instance().hasOffset())
    {
        updateWithMouse();
    }
}

void Camera::_updateProjection()
{
    _proj = glm::perspective(_fov, _ratio, _near, _far);
    _proj[1][1] *= -1;
}

void Camera::_updateView()
{
    _view = glm::lookAt(_position, _position + _front, _up);
}

void Camera::updateWithKeys()
{
    if (Input::instance().pressed(Input::KeyW))
    {
        _position += _movementSpeed * _front;
    }
    if (Input::instance().pressed(Input::KeyS))
    {
        _position -= _movementSpeed * _front;
    }
    if (Input::instance().pressed(Input::KeyD))
    {
        glm::vec3 right = glm::normalize(glm::cross(_front, _up));
        _position += _movementSpeed * right;
    }
    if (Input::instance().pressed(Input::KeyA))
    {
        glm::vec3 right = glm::normalize(glm::cross(_front, _up));
        _position -= _movementSpeed * right;
    }
}

void Camera::updateWithMouse()
{
    auto cursorDelta = Input::instance().cursorOffset();

    yaw += cursorDelta.x;
    pitch += cursorDelta.y;

    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    glm::vec3 front = {
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))};

    _front = glm::normalize(front);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
    return _proj * _view;
}
