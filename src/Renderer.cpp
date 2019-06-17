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
    uint32_t imageCount = swapchain.imageCount;
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

void Renderer::encode(
    const VkQueue &graphicsQueue,
    const uint32_t imageIndex,
    VkCommandBuffer &commandBuffer,
    VkRenderPassBeginInfo &renderPassInfo,
    const VkSemaphore &startSemaphore,
    const VkSemaphore &endSemaphore,
    const VkFence &submissionFence)
{
    VkDeviceSize offsets[] = {0};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    updateUniforms(imageIndex);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _objectPipeline);

    for (auto &object : _objects)
    {
        VkBuffer vertexBuffers[] = {object.vertexBuffer};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _objectPipelineLayout, 0, 1, &object.descriptorSet(imageIndex), 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, object.indicesCount, 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &startSemaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &endSemaphore;

    vkResetFences(_device, 1, &submissionFence);
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, submissionFence);
}

void Renderer::updateUniforms(const uint32_t imageIndex)
{
    VulkanUtilities::UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), (float)_time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), _screenSize[0] / (float)_screenSize[1], 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    // Copy the data in the uniform buffer object to the current uniform buffer.
    void *data;
    vkMapMemory(_device, _uniformBuffersMemory[imageIndex], 0, sizeof(ubo), 0, &data);
    memcpy(data, &ubo, sizeof(ubo));
    vkUnmapMemory(_device, _uniformBuffersMemory[imageIndex]);
}

void Renderer::clean()
{
    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
    vkDestroyPipeline(_device, _objectPipeline, nullptr);
    vkDestroyPipelineLayout(_device, _objectPipelineLayout, nullptr);
    vkDestroySampler(_device, _textureSampler, nullptr);
    vkDestroyDescriptorSetLayout(_device, Object::descriptorSetLayout, nullptr);

    for (size_t i = 0; i < _uniformBuffers.size(); i++)
    {
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
    }

    for (auto &object : _objects)
    {
        object.clean(_device);
    }
}
