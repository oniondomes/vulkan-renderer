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

    VkDeviceSize bufferSize = sizeof(VulkanUtilities::UniformBufferObject);
    _uniformBuffers.resize(imageCount);
    _uniformBuffersMemory.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        VulkanUtilities::createBuffer(
            _device,
            physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            _uniformBuffers[i],
            _uniformBuffersMemory[i]);
    }

    createDescriptorPool(imageCount);

    // Create descriptor sets (one per object)

    for (auto &object : _objects)
    {
        object.generateDescriptorSets(_device, _descriptorPool, _uniformBuffers, _textureSampler, imageCount);
    }
}

void Renderer::createDescriptorPool(uint32_t imageCount)
{
    // Descriptor sets must be allocated from a descriptor pool.
    // It'll be created every frame
    std::array<VkDescriptorPoolSize, 2> poolSizes = {};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(imageCount);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(imageCount);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    // Specify maximum number of descriptor sets that may be allocated.
    poolInfo.maxSets = static_cast<uint32_t>(imageCount);

    if (vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create descriptor pool.");
    }
}

void Renderer::update(const double deltaTime)
{
    _time += deltaTime;
}
