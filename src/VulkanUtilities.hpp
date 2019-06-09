#pragma once

#include "common.hpp"

class VulkanUtilities
{
private:
    static VkDebugReportCallbackEXT debugMessenger;

public:
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        uint32_t presentFamily = -1;
        uint32_t graphicsFamily = -1;

        const bool isComplete() const
        {
            return graphicsFamily >= 0 && presentFamily >= 0;
        }

        const std::set<uint32_t> getIndices() const
        {
            return { presentFamily, graphicsFamily };
        }
    };

    struct SwapchainParameters
    {
        VulkanUtilities::SwapchainSupportDetails support;
        VkExtent2D extent;
        VkSurfaceFormatKHR surface;
        VkPresentModeKHR mode;
        uint32_t imageCount;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            // The number of bytes from one entry to the next.
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

            // Description for position attribute
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            // Description for color attribute
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex &other) const
        {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    static bool enableValidationLayers;

    static int pickPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface, VkPhysicalDevice &device);
    static bool isDeviceSuitable(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static VulkanUtilities::SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static VulkanUtilities::QueueFamilyIndices getGraphicsQueueFamilyIndex(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static int createCommandPool(
        const VkDevice &device,
        VkCommandPool &commandPool,
        VulkanUtilities::QueueFamilyIndices &queueFamilyIndices);
    static int createLogicalDevice(
        const VkPhysicalDevice physicalDevice,
        std::set<u_int32_t> &queuesIndices,
        VkPhysicalDeviceFeatures &deviceFeatures,
        VkDevice &device);
    static VulkanUtilities::SwapchainParameters generateSwapchainParameters(
        VkPhysicalDevice &physicalDevice,
        VkSurfaceKHR &surface,
        const int width,
        const int height);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availableModes);
    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const int width, const int height);

    static std::vector<char> readFile(const std::string &filename);

    static void createBuffer(
        VkDevice &device,
        VkPhysicalDevice &physicalDevice,
        VkDeviceSize size,
        VkBufferUsageFlags flags,
        VkMemoryPropertyFlags propertyFlags,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);

    static void createVertexBuffer(
        std::vector<VulkanUtilities::Vertex> vertices,
        VkDevice &device,
        VkBuffer &vertexBuffer,
        VkPhysicalDevice &physicalDevice,
        VkDeviceMemory &vertexBufferMemory,
        VkCommandPool &commandPool,
        VkQueue &graphicsQueue);

    static void createIndexBuffer(
        std::vector<uint32_t> indices,
        VkDevice &device,
        VkBuffer &indexBuffer,
        VkPhysicalDevice &physicalDevice,
        VkDeviceMemory &indexBufferMemory,
        VkCommandPool &commandPool,
        VkQueue &graphicsQueue);

    static void copyBuffer(
        VkBuffer srcBuffer,
        VkBuffer destBuffer,
        VkDeviceSize size,
        VkDevice &device,
        VkCommandPool &commandPool,
        VkQueue &graphicsQueue);

    static void createUniformBuffers(
        std::vector<VkBuffer> &uniformBuffers,
        std::vector<VkDeviceMemory> &uniformBuffersMemory,
        std::vector<VkImage> &swapchainImages,
        VkDevice &device,
        VkPhysicalDevice &physicalDevice);

    static void createTextureImage(
        VkBuffer &stagingBuffer,
        VkDeviceMemory &stagingBufferMemory,
        VkImage &textureImage,
        VkDeviceMemory &textureImageMemory,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device,
        VkPhysicalDevice &physicalDevice);

    static void createImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory,
        VkDevice &device,
        VkPhysicalDevice &physicalDevice);

    static VkCommandBuffer beginSingleTimeCommands(VkCommandPool &commandPool, VkDevice &device);

    static void endSingleTimeCommands(
        VkCommandBuffer commandBuffer,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device);

    static void copyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device);

    static void transitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device);

    static VkImageView createImageView(
        VkImage &image,
        VkFormat const &format,
        VkImageAspectFlags aspectFlags,
        VkDevice &device);

    static void createTextureImageView(
        VkImageView &textureImageView,
        VkImage &textureImage,
        VkDevice &device);

    static void createTextureSampler(VkSampler &textureSampler, VkDevice &device);

    static void createDepthResources(
        VkImage &depthImage,
        VkDeviceMemory &depthImageMemory,
        VkImageView &depthImageView,
        VkExtent2D &swapChainExtent,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device,
        VkPhysicalDevice &physicalDevice);

    static VkFormat findSupportedFormat(
        const std::vector<VkFormat> &candidates,
        VkImageTiling tiling,
        VkFormatFeatureFlags features,
        VkPhysicalDevice &physicalDevice);

    static VkFormat findDepthFormat(VkPhysicalDevice &physicalDevice);

    static bool checkValidationLayerSupport(std::vector<const char *> validationLayers);

    static void createInstance(VkInstance &instance, bool debugEnabled);

    static std::vector<const char *> getRequiredExtensions(bool enableValidationLayers);

    static void createSwapchain(
        VulkanUtilities::SwapchainParameters &parameters,
        VkSurfaceKHR &surface,
        VkDevice &device,
        VulkanUtilities::QueueFamilyIndices &queues,
        VkSwapchainKHR &swapchain,
        VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
};
