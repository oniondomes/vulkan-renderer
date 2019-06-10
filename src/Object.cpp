#include "Object.hpp"
#include "MeshUtilities.hpp"
#include "VulkanUtilities.hpp"

VkDescriptorSetLayout Object::descriptorSetLayout = VK_NULL_HANDLE;

Object::Object(std::string &name, const std::string *path)
{
    _name = name;
    _path = path;
    params.model = glm::mat4(0.0f);
}

Object::~Object()
{
}

void Object::load(
    VkPhysicalDevice &physicalDevice,
    VkDevice &device,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    Mesh mesh;
    MeshUtilities::loadMesh(*_path, mesh);
    indicesCount = static_cast<uint32_t>(mesh.indices.size());

    VulkanUtilities::createVertexBuffer(
        mesh,
        device,
        vertexBuffer,
        physicalDevice,
        _vertexBufferMemory,
        commandPool,
        graphicsQueue);

    VulkanUtilities::createIndexBuffer(
        mesh.indices,
        device,
        indexBuffer,
        physicalDevice,
        _indexBufferMemory,
        commandPool,
        graphicsQueue);

    // TODO: Can I cast this inline?
    std::string path = "./resources/textures/cube.png";

    VulkanUtilities::createTextureImage(
        path,
        _stagingBuffer,
        _stagingBufferMemory,
        _textureImage,
        _textureImageMemory,
        graphicsQueue,
        commandPool,
        device,
        physicalDevice);
}

void Object::createDescriptorSetLayout(VkDevice &device, VkSampler &textureSampler)
{
    // Describe binding for ubo used in the shader
    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;

    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    // Sampler binding
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

    // Bindings must be combined into a VkDescriptorSetLayout object
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create descriptor set layout.");
    }
}
