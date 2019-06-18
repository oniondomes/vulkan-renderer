#ifndef Swapchain_hpp
#define Swapchain_hpp

#include "common.hpp"
#include "VulkanUtilities.hpp"

class Swapchain
{
public:
    Swapchain();
    ~Swapchain();

    void init(VkInstance &instance, VkSurfaceKHR &surface, const int width, const int height);
    void setup(const int width, const int height);
    void unset();
    void clean();
    void createSyncObjects();
    void createRenderPass();
    void resize(const int width, const int height);
    VkResult commit();
    VkResult run(VkRenderPassBeginInfo &info);

    void step() { currentFrame = (currentFrame + 1) % imageCount; }
    VkCommandBuffer &getCommandBuffer() { return _commandBuffers[imageIndex]; }
    VkSemaphore &getStartSemaphore() { return _imageAvailableSemaphores[currentFrame]; }
    VkSemaphore &getEndSemaphore() { return _renderFinishedSemaphores[currentFrame]; }
    VkFence &getFence() { return _inFlightFences[currentFrame]; }

    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkCommandPool commandPool;
    VkRenderPass renderPass;

    VulkanUtilities::SwapchainParameters parameters;

    uint32_t currentFrame = 0;
    uint32_t imageIndex = 0;
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