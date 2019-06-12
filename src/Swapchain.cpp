#include "Swapchain.hpp"
#include "VulkanUtilities.hpp"

Swapchain::Swapchain()
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::init(VkInstance &instance, VkSurfaceKHR &surface, const int width, const int height)
{
    _surface = surface;
    currentFrame = 0;

    VulkanUtilities::pickPhysicalDevice(instance, surface, physicalDevice);

    VulkanUtilities::QueueFamilyIndices queues = VulkanUtilities::getGraphicsQueueFamilyIndex(physicalDevice, surface);
    std::set<uint32_t> queueIndices = queues.getIndices();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VulkanUtilities::createLogicalDevice(physicalDevice, queueIndices, deviceFeatures, device);
    // Retrieve references to the queues
    vkGetDeviceQueue(device, queues.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, queues.presentFamily, 0, &_presentQueue);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queues.graphicsFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create command pool.");
    }

    setup(width, height);

    createSyncObjects();
}

void Swapchain::setup(const int width, const int height)
{
    VulkanUtilities::QueueFamilyIndices queues = VulkanUtilities::getGraphicsQueueFamilyIndex(physicalDevice, _surface);

    parameters = VulkanUtilities::generateSwapchainParameters(physicalDevice, _surface, width, height);
    maxFramesInFlight = parameters.imageCount;
    VulkanUtilities::createSwapchain(parameters, _surface, device, queues, _swapchain);

    // Render pass.
    createRenderPass();

    VkFormat depthFormat = VulkanUtilities::findDepthFormat(physicalDevice);
    VulkanUtilities::createDepthResources(
        _depthImage,
        _depthImageMemory,
        _depthImageView,
        parameters.extent,
        graphicsQueue,
        commandPool,
        device,
        physicalDevice);

    // Obtain presentable images from associated with the swapchain.
    vkGetSwapchainImagesKHR(device, _swapchain, &parameters.imageCount, nullptr);
    imageCount = parameters.imageCount;
    _swapchainImages.resize(imageCount);
    std::cout << "Swapchain using " << imageCount << " images." << std::endl;
    vkGetSwapchainImagesKHR(device, _swapchain, &parameters.imageCount, _swapchainImages.data());

    // Create a view for each image.
    _swapchainImageViews.resize(imageCount);
    for (size_t i = 0; i < imageCount; i++)
    {
        _swapchainImageViews[i] = VulkanUtilities::createImageView(
            _swapchainImages[i],
            parameters.surface.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            device);
    }

    // Create swapchain framebuffers.
    _swapchainFramebuffers.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++)
    {
        std::array<VkImageView, 2> attachments = {_swapchainImageViews[i], _depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = parameters.extent.width;
        framebufferInfo.height = parameters.extent.height;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create swap framebuffers.");
        }
    }

    // Create command buffers.

    _commandBuffers.resize(_swapchainFramebuffers.size());
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to allocate command buffers.");
    }

}

void Swapchain::unset()
{
}

void Swapchain::createSyncObjects()
{
    _imageAvailableSemaphores.resize(maxFramesInFlight);
    _renderFinishedSemaphores.resize(maxFramesInFlight);
    _inFlightFences.resize(maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < maxFramesInFlight; i++)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create semaphores and fences.");
        }
    }
}

// TODO: What the hell is render pass?
void Swapchain::createRenderPass()
{
    // Color attachment.
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = parameters.surface.format;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth attachment.
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = VulkanUtilities::findDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachemntRef = {};
    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Subpass.
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Dependencies.

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render pass.
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Unable to create render pass.");
    }
}

void Swapchain::resize(const int width, const int height)
{
    if (width == parameters.extent.width && height == parameters.extent.height)
    {
        return;
    }

    vkDeviceWaitIdle(device);
    unset();
    setup(width, height);
}
