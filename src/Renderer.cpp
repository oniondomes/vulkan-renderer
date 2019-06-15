#include "Renderer.hpp"
#include "Pipeline.hpp"

// Resources paths.
const std::string MODEL_PATH = "./resources/models/cube.obj";
const std::string TEXTURE_PATH = "./resources/textures/cube.png";
const std::string VERT_SHADER_PATH = "./resources/shaders/vert.spv";
const std::string FRAG_SHADER_PATH = "./resources/shaders/frag.spv";

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::init(Swapchain &swapchain, const int width, const int height)
{
    // TODO: These can be constant
    auto &physicalDevice = swapchain.physicalDevice;
    auto &commandPool = swapchain.commandPool;
    auto &renderPass = swapchain.renderPass;
    auto &graphicsQueue = swapchain.graphicsQueue;
    uint32_t imageCount = swapchain.maxFramesInFlight;
    _device = swapchain.device;


    std::string name("cube");
    Object cube(name, &MODEL_PATH);

    _objects.emplace_back(cube);

    _screenSize = glm::vec2(width, height);

    VulkanUtilities::createTextureSampler(_textureSampler, _device);

    for (auto &object : _objects)
    {
        object.load(physicalDevice, _device, commandPool, graphicsQueue);
    }

    Object::createDescriptorSetLayout(_device, _textureSampler);

    Pipeline::create(
        _device,
        swapchain,
        _screenSize[0],
        _screenSize[1],
        Object::descriptorSetLayout,
        _objectPipelineLayout,
        renderPass,
        _objectPipeline);
}
