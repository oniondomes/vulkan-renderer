#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <string>
#include <set>
#include "optional.hpp"

const int WIDTH = 640;
const int HEIGHT = 480;

struct QueueFamilyIndices
{
    std::experimental::optional<uint32_t> presentFamily;
    std::experimental::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
