#include <fstream>
#include "VulkanUtilities.hpp"

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

bool VulkanUtilities::enableValidationLayers = true;

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
            static_cast<uint32_t>(height)
        };

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

int VulkanUtilities::pickPhysicalDevice(VkInstance &instance, VkSurfaceKHR &surface, VkPhysicalDevice &physicalDevice)
{
    // Reset physical device.
    physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vilkan support!");
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
        throw std::runtime_error("failed to find suitable GPU!");
    }

    return 0;
}

QueueFamilyIndices VulkanUtilities::getGraphicsQueueFamilyIndex(const VkPhysicalDevice &device, VkSurfaceKHR &surface)
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
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
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
    if (params.support.capabilities.maxImageCount > 0 && imageCount > params.support.capabilities.maxImageCount) {
        imageCount = params.support.capabilities.maxImageCount;
    }

    params.imageCount = imageCount;
    return params;
};
