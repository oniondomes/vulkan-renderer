#ifndef Swapchain_hpp
#define Swapchain_hpp

#include "common.hpp"
#include "VulkanUtilities.hpp"

class Swapchain
{
public:
    Swapchain();

    void init(VkInstance &instance, VkSurfaceKHR &surface, const int width, const int height);
    void setup(const int width, const int height);
    void unset();
    void createSyncObjects();
    void createRenderPass();
    void resize(const int width, const int height);

    ~Swapchain();

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    VkFormat swapchainImageFormat;
    VkRenderPass renderPass;

    VulkanUtilities::SwapchainParameters parameters;

    uint32_t maxFramesInFlight;
    uint32_t currentFrame;

private:
    VkSwapchainKHR _swapchain;
    VkSurfaceKHR _surface;
    VkQueue _presentQueue;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    std::vector<VkCommandBuffer> _commandBuffers;
};

#endif