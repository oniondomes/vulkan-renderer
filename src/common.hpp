#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <set>
#include <optional.hpp>

const int WIDTH = 480;
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
