#pragma once

#include "MeshUtilities.hpp"
#include "common.hpp"

class VulkanUtilities
{
private:
    static VkDebugUtilsMessengerEXT _debugMessenger;
    static VkDeviceSize _minUniformBufferOffsetAlignment;

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

    struct ObjectInfo {
        glm::mat4 model;
    };

    struct CameraInfo
    {
        glm::mat4 viewProjection;
    };

    struct LightInfo
    {
        glm::vec3 direction;
    };

    static bool enableValidationLayers;

    static int pickPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface, VkPhysicalDevice &device);
    static bool isDeviceSuitable(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static VulkanUtilities::SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static VulkanUtilities::QueueFamilyIndices getGraphicsQueueFamilyIndex(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static void createLogicalDevice(
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
        Mesh &mesh,
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

    static void createTextureImage(
        std::string &path,
        VkBuffer &stagingBuffer,
        VkDeviceMemory &stagingBufferMemory,
        VkImage &textureImage,
        VkDeviceMemory &textureImageMemory,
        VkQueue &graphicsQueue,
        VkCommandPool &commandPool,
        VkDevice &device,
        VkPhysicalDevice &physicalDevice);

    static void createImage(
        VkPhysicalDevice &physicalDevice,
        VkDevice &device,
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);

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

    static VkDeviceSize nextOffset(size_t size);
};
