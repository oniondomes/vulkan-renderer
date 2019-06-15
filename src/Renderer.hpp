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
    void createDescriptorPool(uint32_t imageCount);

    ~Renderer();

private:
    VkDevice _device;
    VkSampler _textureSampler;

    glm::vec2 _screenSize;
    std::vector<Object> _objects;
    VkDescriptorPool _descriptorPool;

    // Pipelines.
    VkPipelineLayout _objectPipelineLayout;
    VkPipeline _objectPipeline;

    // Per frame data.
    std::vector<VkBuffer> _uniformBuffers;
    std::vector<VkDeviceMemory> _uniformBuffersMemory;
};

#endif
