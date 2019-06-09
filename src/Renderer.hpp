#ifndef Renderer_hpp
#define Renderer_hpp

#include "Swapchain.hpp"
#include "MeshUtilities.hpp"
#include "Object.hpp"

class Renderer
{
public:
    Renderer();

    void init(Swapchain &swapchain, int const width, int const heigth);

    ~Renderer();
private:
    VkDevice _device;

    glm::vec2 _screenSize;
    std::vector<Object> _objects;
};

#endif
