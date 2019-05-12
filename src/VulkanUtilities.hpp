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
};
