#include "Input.hpp"

Input &Input::instance()
{
    static Input input;

    return input;
};

void Input::resizeEvent(const uint32_t width, const uint32_t height)
{
    _width = width > 0 ? width : 1;
    _height = height > 0 ? height : 1;
    _resized = true;
}

void Input::keyDownEvent(int key, int action)
{
    if (key == GLFW_KEY_UNKNOWN)
    {
        return;
    }

    if (action == GLFW_PRESS)
    {
        _keys[key].pressed = true;
    }
    else if (action == GLFW_RELEASE)
    {
        _keys[key].pressed = false;
    }
}

void Input::mouseMoveEvent(double xPos, double yPos)
{
    if (!_cursorSet)
    {
        _lastCursorX = xPos;
        _lastCursorY = yPos;
        _cursorSet = true;
    }

    float xOffset = xPos - _lastCursorX;
    float yOffset = _lastCursorY - yPos;
    _lastCursorX = xPos;
    _lastCursorY = yPos;

    xOffset *= _sensitivity;
    yOffset *= _sensitivity;

    _cursorOffset.x = xOffset;
    _cursorOffset.y = yOffset;

    std::cout << _cursorOffset.x << " : " ;
    std::cout << _cursorOffset.y << std::endl;
}

bool Input::hasOffset() const {
    return _cursorOffset.x != 0.0f || _cursorOffset.y != 0.0f;
}

bool Input::pressed(const Key &keyboardKey) const
{
    return _keys[keyboardKey].pressed;
}

void Input::update()
{
    _resized = false;
    _cursorOffset.x = 0;
    _cursorOffset.y = 0;

    glfwPollEvents();
}
