#pragma once

#include "common.hpp"

class VulkanUtilities
{
public:
    static int pickPhysicalDevice(VkInstance& instance, VkSurfaceKHR& surface, VkPhysicalDevice& device);
    static bool isDeviceSuitable(const VkPhysicalDevice& device, VkSurfaceKHR& surface);
    static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, VkSurfaceKHR& surface);
    static QueueFamilyIndices getGraphicsQueueFamilyIndex(const VkPhysicalDevice& device, VkSurfaceKHR& surface);

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    static std::vector<char> readFile(const std::string& filename);
};
