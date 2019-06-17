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

    VulkanUtilities::createTextureImageView(
        _textureImageView,
        _textureImage,
        device);
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

void Object::generateDescriptorSets(
    const VkDevice &device,
    const VkDescriptorPool &descriptorPool,
    const std::vector<VkBuffer> &uniformBuffers,
    VkSampler &textureSampler,
    uint32_t imageCount)
{
    _descriptorSets.resize(imageCount);
    std::vector<VkDescriptorSetLayout> layouts(imageCount, descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(VulkanUtilities::UniformBufferObject);

        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = _textureImageView;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = _descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void Object::clean(VkDevice &device)
{
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);

    vkFreeMemory(device, _vertexBufferMemory, nullptr);
    vkFreeMemory(device, _indexBufferMemory, nullptr);
    vkFreeMemory(device, _textureImageMemory, nullptr);

    vkDestroyImageView(device, _textureImageView, nullptr);
    vkDestroyImage(device, _textureImage, nullptr);
}
