#include "VulkanUtilities.hpp"

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

class VulkanApp
{
private:
    GLFWwindow *window;
    VkInstance instance;
    VkSwapchainKHR swapchain;

public:
    void mainLoop()
    {
        while (!glfwWindowShouldClose(window))
        {
            // Check user input

            // Check window events
            glfwPollEvents();

            // Draw frame
        }

        // Need device to a function
        // vkDeviceWaitIdle();
    }

    void cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createWindow()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("Unable to initialize GLFW.");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);

        if (!window)
        {
            throw std::runtime_error("Unable to create GLFW window.");
        }
    }

    void initVulkan()
    {
        // Create Vulkan instance
        if (enableValidationLayers && !VulkanUtilities::checkValidationLayerSupport(validationLayers))
        {
            std::cerr << "validation layers requested, but not available!" << std::endl;
            enableValidationLayers = false;
        } else {
            std::cerr << "Validation layers enabled" << std::endl;
        }

        VulkanUtilities::createInstance(instance, enableValidationLayers);
    }

    void createSwapchain()
    {
    }

    void run()
    {
        createWindow();
        initVulkan();
        createSwapchain();
        mainLoop();
        cleanup();
    }
};

int main()
{
    VulkanApp app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
