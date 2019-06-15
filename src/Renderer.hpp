#ifndef Renderer_hpp
#define Renderer_hpp

#include "Swapchain.hpp"
#include "MeshUtilities.hpp"
#include "Object.hpp"

class Renderer
{
public:
    Renderer();

    void init(Swapchain &swapchain, const int width, const int heigth);

    ~Renderer();

private:
    VkDevice _device;
    VkSampler _textureSampler;

    glm::vec2 _screenSize;
    std::vector<Object> _objects;

    // Pipelines.
    VkPipelineLayout _objectPipelineLayout;
    VkPipeline _objectPipeline;
};

#endif
