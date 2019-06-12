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
    VkResult run(VkRenderPassBeginInfo &info);
    void resize(const int width, const int height);

    ~Swapchain();

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    VkRenderPass renderPass;

    VulkanUtilities::SwapchainParameters parameters;

    uint32_t maxFramesInFlight;
    uint32_t currentFrame;

    uint32_t imageIndex;

    // Number of presentable images.
    uint32_t imageCount;

private:
    VkSwapchainKHR _swapchain;
    VkSurfaceKHR _surface;
    VkQueue _presentQueue;

    // Sync object.
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    // Depth image.
    VkImage _depthImage;
    VkDeviceMemory _depthImageMemory;
    VkImageView _depthImageView;

    // Swapchain images.
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;

    // Buffers.
    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkFramebuffer> _swapchainFramebuffers;
};

#endif