#pragma once

#include "common.hpp"

class VulkanUtilities
{
public:
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
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

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            // The number of bytes from one entry to the next.
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

            // Description for position attribute
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            // Description for color attribute
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            return attributeDescriptions;
        }
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    static bool enableValidationLayers;

    static int pickPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface, VkPhysicalDevice &device);
    static bool isDeviceSuitable(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static VulkanUtilities::SwapchainSupportDetails querySwapchainSupport(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static QueueFamilyIndices getGraphicsQueueFamilyIndex(const VkPhysicalDevice &device, VkSurfaceKHR &surface);
    static int createCommandPool(const VkDevice &device, VkCommandPool &commandPool, QueueFamilyIndices &queueFamilyIndices);
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
        std::vector<uint16_t> indices,
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
};
