#include <fstream>
#include "VulkanUtilities.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

bool VulkanUtilities::enableValidationLayers = true;
VkDebugReportCallbackEXT VulkanUtilities::debugMessenger;

bool checkDeviceExtensionsSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto &extension : extensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

uint32_t findMemoryType(VkPhysicalDevice &physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // This object will have to array fields:
    // - memoryTypes
    // - memoryHeaps
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    // TODO: What the fuck is going on here?
    // Figure this out later
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Unable to find suitable memory type.");
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkCommandBuffer VulkanUtilities::beginSingleTimeCommands(VkCommandPool &commandPool, VkDevice &device)
{
    // Allocation params for command buffer object.
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    // Write passed commands from command pool into commandBuffer
    // Actually, I just allocate memory for this
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    // Start recording command buffer
    // beginInfo contains command poll
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // So now I can return a buffer that contains commands
    // from the command pool
    // Do all of this mean that a copied the to command to buffer?
    return commandBuffer;
}

void VulkanUtilities::endSingleTimeCommands(
    VkCommandBuffer commandBuffer,
    VkQueue &graphicsQueue,
    VkCommandPool &commandPool,
    VkDevice &device)
{
    // End recording command buffer
    vkEndCommandBuffer(commandBuffer);

    // Prepare info to submit the buffer to the queue
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    // Wait for a queue to finish executing its commands
    vkQueueWaitIdle(graphicsQueue);

    // Free command buffer memory
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkSurfaceFormatKHR VulkanUtilities::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        return {VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanUtilities::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes)
{
    VkPresentModeKHR bestFitMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto &availableMode : availableModes)
    {
        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availableMode;
        }
        else if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            bestFitMode = availableMode;
        }
    }

    return bestFitMode;
}

VkExtent2D VulkanUtilities::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const int width, const int height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

std::vector<char> VulkanUtilities::readFile(const std::string &filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

VulkanUtilities::SwapchainSupportDetails VulkanUtilities::querySwapchainSupport(
    const VkPhysicalDevice &device,
    VkSurfaceKHR &surface)
{
    VulkanUtilities::SwapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool VulkanUtilities::isDeviceSuitable(const VkPhysicalDevice &device, VkSurfaceKHR &surface)
{
    bool isComplete = VulkanUtilities::getGraphicsQueueFamilyIndex(device, surface).isComplete();
    bool extensionsSupported = checkDeviceExtensionsSupport(device);
    bool swapChainAdequate = false;

    VkPhysicalDeviceFeatures supportedDeviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedDeviceFeatures);

    if (extensionsSupported)
    {
        VulkanUtilities::SwapchainSupportDetails swapChainSupport = VulkanUtilities::querySwapchainSupport(device, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return extensionsSupported && isComplete && swapChainAdequate && supportedDeviceFeatures.samplerAnisotropy;
}

int VulkanUtilities::pickPhysicalDevice(
    VkInstance &instance,
    VkSurfaceKHR &surface,
    VkPhysicalDevice &physicalDevice)
{
    // Reset physical device.
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Unable to find GPUs with Vilkan support.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto &deviceCandidate : devices)
    {
        if (isDeviceSuitable(deviceCandidate, surface))
        {
            physicalDevice = deviceCandidate;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Unable to find suitable GPU.");
    }

    return 0;
}

VulkanUtilities::QueueFamilyIndices VulkanUtilities::getGraphicsQueueFamilyIndex(
    const VkPhysicalDevice &device,
    VkSurfaceKHR &surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;

    for (const auto &queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamily.queueCount > 0 && presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

int VulkanUtilities::createLogicalDevice(
    const VkPhysicalDevice physicalDevice,
    std::set<u_int32_t> &queuesIndices,
    VkPhysicalDeviceFeatures &deviceFeatures,
    VkDevice &device)
{
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (uint32_t queueFamilyIndex : queuesIndices)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Device setup.
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
    {
        std::cerr << "Unable to create logical Vulkan device." << std::endl;
        return 3;
    }
    return 0;
}

int VulkanUtilities::createCommandPool(
    const VkDevice &device,
    VkCommandPool &commandPool,
    QueueFamilyIndices &queueFamilyIndices)
{
    VkCommandPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    createInfo.flags = 0;

    if (vkCreateCommandPool(device, &createInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create command pool!");
        return 3;
    }

    return 0;
}

VulkanUtilities::SwapchainParameters VulkanUtilities::generateSwapchainParameters(
    VkPhysicalDevice &physicalDevice,
    VkSurfaceKHR &surface,
    const int width,
    const int height)
{
    VulkanUtilities::SwapchainParameters params;

    params.support = VulkanUtilities::querySwapchainSupport(physicalDevice, surface);
    params.extent = VulkanUtilities::chooseSwapExtent(params.support.capabilities, width, height);
    params.surface = VulkanUtilities::chooseSwapSurfaceFormat(params.support.formats);
    params.mode = VulkanUtilities::chooseSwapPresentMode(params.support.presentModes);

    uint32_t imageCount = params.support.capabilities.minImageCount + 1;
    if (params.support.capabilities.maxImageCount > 0 && imageCount > params.support.capabilities.maxImageCount)
    {
        imageCount = params.support.capabilities.maxImageCount;
    }

    params.imageCount = imageCount;
    return params;
};

void VulkanUtilities::createVertexBuffer(
    Mesh &mesh,
    VkDevice &device,
    VkBuffer &vertexBuffer,
    VkPhysicalDevice &physicalDevice,
    VkDeviceMemory &vertexBufferMemory,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    VkDeviceSize bufferSize = sizeof(mesh.vertices[0]) * mesh.vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanUtilities::createBuffer(
        device,
        physicalDevice,
        bufferSize,
        // Such buffer can be used as source in a memory transfer operation.
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, mesh.vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VulkanUtilities::createBuffer(
        device,
        physicalDevice,
        bufferSize,
        // Buffer can be used as destination in a memory transfer operation.
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        // Property flag meaning the buffer is located in device local memory
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer,
        vertexBufferMemory);

    VulkanUtilities::copyBuffer(stagingBuffer, vertexBuffer, bufferSize, device, commandPool, graphicsQueue);

    // Destroying staging buffer object.
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
};

void VulkanUtilities::createBuffer(
    VkDevice &device,
    VkPhysicalDevice &physicalDevice,
    VkDeviceSize size,
    VkBufferUsageFlags usageFlags,
    VkMemoryPropertyFlags propertyFlags,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usageFlags;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create a buffer");
    }

    VkMemoryRequirements memRequirements;

    // Getting memory requirement for a created buffer
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocationInfo = {};
    allocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocationInfo.allocationSize = memRequirements.size;
    allocationInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, propertyFlags);

    if (vkAllocateMemory(device, &allocationInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate buffer memory");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanUtilities::createIndexBuffer(
    std::vector<uint32_t> indices,
    VkDevice &device,
    VkBuffer &indexBuffer,
    VkPhysicalDevice &physicalDevice,
    VkDeviceMemory &indexBufferMemory,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VulkanUtilities::createBuffer(
        device,
        physicalDevice,
        bufferSize,
        // Use buffer as a transfer source.
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VulkanUtilities::createBuffer(
        device,
        physicalDevice,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer,
        indexBufferMemory);

    VulkanUtilities::copyBuffer(stagingBuffer, indexBuffer, bufferSize, device, commandPool, graphicsQueue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanUtilities::createUniformBuffers(
    std::vector<VkBuffer> &uniformBuffers,
    std::vector<VkDeviceMemory> &uniformBuffersMemory,
    std::vector<VkImage> &swapchainImages,
    VkDevice &device,
    VkPhysicalDevice &physicalDevice)
{
    VkDeviceSize bufferSize = sizeof(VulkanUtilities::UniformBufferObject);

    uniformBuffers.resize(swapchainImages.size());
    uniformBuffersMemory.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); i++)
    {
        VulkanUtilities::createBuffer(
            device,
            physicalDevice,
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformBuffers[i],
            uniformBuffersMemory[i]);
    }
}

void VulkanUtilities::transitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkQueue &graphicsQueue,
    VkCommandPool &commandPool,
    VkDevice &device)
{
    // Create a command buffer from a command pool
    VkCommandBuffer commandBuffer = VulkanUtilities::beginSingleTimeCommands(
        commandPool,
        device);

    // ImageMemoryBarrier will ensure that writing to a buffer will be
    // completed before anyone would read from it
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    // Source stage specifies the pipeline stage that should
    // happen before the barrier
    VkPipelineStageFlags sourceStage;

    // Destination stage defines the pipeline stage in which
    // operations will wait on the barrier
    VkPipelineStageFlags destinationStage;

    // Here we need to handle to types of transition
    // - undefined -> transfer
    // - transfer -> shader reading (here shader reading needs to wait for transfer to finish)

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    VulkanUtilities::endSingleTimeCommands(
        commandBuffer,
        graphicsQueue,
        commandPool,
        device);
}

void VulkanUtilities::createTextureImage(
    std::string &path,
    VkBuffer &stagingBuffer,
    VkDeviceMemory &stagingBufferMemory,
    VkImage &textureImage,
    VkDeviceMemory &textureImageMemory,
    VkQueue &graphicsQueue,
    VkCommandPool &commandPool,
    VkDevice &device,
    VkPhysicalDevice &physicalDevice)
{
    int textureWidth, textureHeight, textureChannel;

    // Get an array of pixels
    stbi_uc *pixels = stbi_load(
        path.c_str(),
        &textureWidth,
        &textureHeight,
        &textureChannel,
        STBI_rgb_alpha);

    // Calculate image size
    VkDeviceSize imageSize = textureWidth * textureWidth * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    // Create buffer of imageSize size
    // It would be staging buffer
    // -------
    // Does the memory object allow to connect
    // host memory to a buffer?
    VulkanUtilities::createBuffer(
        device,
        physicalDevice,
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Data is pointing to a pointer to the beginning
    // of the stagingBufferMemory
    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingBufferMemory);

    // Here I just wrote pixels data into stagingBufferMemory
    // Now I need to clear original pixels
    stbi_image_free(pixels);

    // At the end I have a buffer with some pixels data
    // The we'll create an image object
    // > to do what???
    // My guess is that later we will connect created image object to
    // a buffer (or just copy bytes from it).
    VulkanUtilities::createImage(
        textureWidth,
        textureHeight,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage,
        textureImageMemory,
        device,
        physicalDevice);

    // Now I need to copy stagingBuffer to textureImage

    VulkanUtilities::transitionImageLayout(
        textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        graphicsQueue,
        commandPool,
        device);
    VulkanUtilities::copyBufferToImage(
        stagingBuffer,
        textureImage,
        static_cast<uint32_t>(textureWidth),
        static_cast<uint32_t>(textureHeight),
        graphicsQueue,
        commandPool,
        device);

    transitionImageLayout(
        textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        graphicsQueue,
        commandPool,
        device);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanUtilities::createImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage &image,
    VkDeviceMemory &imageMemory,
    VkDevice &device,
    VkPhysicalDevice &physicalDevice)
{
    // Set image object info
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    // Acquire memory requirements considering images size
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    // Fill in info for memory allocation
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

    // Allocate memory and binding image object to device memory
    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);

    // As the result we have an image object with bounded memory with
    // specific characterics
}

void VulkanUtilities::copyBuffer(
    VkBuffer srcBuffer,
    VkBuffer destBuffer,
    VkDeviceSize size,
    VkDevice &device,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    // Specify which part of the buffer will
    // be copied to another buffer
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, destBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}

void VulkanUtilities::copyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height,
    VkQueue &graphicsQueue,
    VkCommandPool &commandPool,
    VkDevice &device)
{
    // Put commands in command pool to command buffer
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(commandPool, device);

    // Specify which part of the buffer to which
    // part of the image object will be written
    VkBufferImageCopy region = {};

    region.bufferOffset = 0;

    // Specify how the pixels are layed out in the memory
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    // To which part of the image I want to write buffer data
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    // Enqueue buffer to image copy operation
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    // Submit buffer into the queue
    endSingleTimeCommands(commandBuffer, graphicsQueue, commandPool, device);
}

VkImageView VulkanUtilities::createImageView(
    VkImage &image,
    VkFormat const &format,
    VkImageAspectFlags aspectFlags,
    VkDevice &device)
{
    VkImageView imageView;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create texture image view");
    }

    return imageView;
}

void VulkanUtilities::createTextureImageView(
    VkImageView &textureImageView,
    VkImage &textureImage,
    VkDevice &device)
{
    textureImageView = VulkanUtilities::createImageView(
        textureImage,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_ASPECT_COLOR_BIT,
        device);
}

void VulkanUtilities::createTextureSampler(VkSampler &textureSampler, VkDevice &device)
{
    VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.magFilter = VK_FILTER_NEAREST;
    createInfo.minFilter = VK_FILTER_NEAREST;

    createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    createInfo.anisotropyEnable = VK_TRUE;
    createInfo.maxAnisotropy = 16;
    createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.mipLodBias = 0.0f;
    createInfo.minLod = 0.0f;
    createInfo.maxLod = 0.0f;

    if (vkCreateSampler(device, &createInfo, nullptr, &textureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkFormat VulkanUtilities::findSupportedFormat(
    const std::vector<VkFormat> &candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features,
    VkPhysicalDevice &physicalDevice)
{
    for (VkFormat format : candidates)
    {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VulkanUtilities::findDepthFormat(VkPhysicalDevice &physicalDevice)
{
    return VulkanUtilities::findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physicalDevice);
}

void VulkanUtilities::createDepthResources(
    VkImage &depthImage,
    VkDeviceMemory &depthImageMemory,
    VkImageView &depthImageView,
    VkExtent2D &swapChainExtent,
    VkQueue &graphicsQueue,
    VkCommandPool &commandPool,
    VkDevice &device,
    VkPhysicalDevice &physicalDevice)
{
    VkFormat depthFormat = findDepthFormat(physicalDevice);

    VulkanUtilities::createImage(
        swapChainExtent.width,
        swapChainExtent.height,
        depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage,
        depthImageMemory,
        device,
        physicalDevice);

    depthImageView = VulkanUtilities::createImageView(
        depthImage,
        depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        device);

    VulkanUtilities::transitionImageLayout(
        depthImage,
        depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        graphicsQueue,
        commandPool,
        device);
}

bool VulkanUtilities::checkValidationLayerSupport(std::vector<const char *> validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto &layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

std::vector<const char *> VulkanUtilities::getRequiredExtensions(bool enableValidationLayers)
{
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char *layerPrefix,
    const char *msg,
    void *userData)
{
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanUtilities::createInstance(VkInstance &instance, bool enableValidationLayers)
{
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = VulkanUtilities::getRequiredExtensions(enableValidationLayers);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create Vulkan instance.");
    }

    if (!enableValidationLayers)
    {
        return;
    }

    VkDebugReportCallbackCreateInfoEXT createCallbackInfo = {};
    createCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    createCallbackInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    createCallbackInfo.pfnCallback = &debugCallback;

    if (CreateDebugReportCallbackEXT(instance, &createCallbackInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to setup debug report callback.");
    }
}

void VulkanUtilities::createSwapchain(
    VulkanUtilities::SwapchainParameters &parameters,
    VkSurfaceKHR &surface,
    VkDevice &device,
    VulkanUtilities::QueueFamilyIndices &queues,
    VkSwapchainKHR &swapchain,
    VkSwapchainKHR oldSwapchain)
{
    if (parameters.support.capabilities.maxImageCount > 0 && parameters.imageCount > parameters.support.capabilities.maxImageCount)
    {
        parameters.imageCount = parameters.support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = parameters.imageCount;
    createInfo.imageFormat = parameters.surface.format;
    createInfo.imageColorSpace = parameters.surface.colorSpace;
    createInfo.imageExtent = parameters.extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = { (uint32_t)(queues.graphicsFamily), (uint32_t)(queues.presentFamily) };

    if (queues.graphicsFamily != queues.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = parameters.support.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = parameters.mode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create swapchain.");
    }
};