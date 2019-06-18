#ifndef Input_hpp
#define Input_hpp

#include "common.hpp"

class Input
{
public:
    static Input &input;
    static Input &instance();

    bool isResized() { return _resized; };

    void resizeEvent(const uint32_t width, const uint32_t height);
    void update();

private:
    uint32_t _width = 1;
    uint32_t _height = 1;
    bool _resized = false;

private:
    Input(){};
    ~Input(){};
    Input(Input const &) = delete;
    void operator=(Input const &) = delete;
};

#endif
