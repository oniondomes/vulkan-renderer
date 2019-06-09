#include "Renderer.hpp"

// Resources paths.
std::string MODEL_PATH = "./resources/models/cube.obj";
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
    const auto & physicalDevice = swapchain.physicalDevice;
    const auto & commandPool = swapchain.commandPool;
    const auto & finalRenderPass = swapchain.renderPass;
    const auto & graphicsQueue = swapchain.graphicsQueue;
    const uint32_t imageCount = swapchain.maxFramesInFlight;
    _device = swapchain.device;


    std::string name("cube");
    Object cube(name, MODEL_PATH);

    _objects.emplace_back(cube);

    _screenSize = glm::vec2(width, height);
}
