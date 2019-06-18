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

void Input::update()
{
    _resized = false;
    glfwPollEvents();
}
