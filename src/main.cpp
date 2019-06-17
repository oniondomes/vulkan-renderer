#include "VulkanUtilities.hpp"
#include "Swapchain.hpp"
#include "Renderer.hpp"

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

// Surface params.
const int WIDTH = 480;
const int HEIGHT = 480;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"
};

class VulkanApp
{
private:
    int width;
    int height;

    GLFWwindow *window;

    VkInstance instance;
    VkSurfaceKHR surface;
    Swapchain swapchain;
    Renderer renderer;

    VkRenderPassBeginInfo renderPassInfo;

public:
    void mainLoop()
    {
        double timer = glfwGetTime();

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            double currentTime = glfwGetTime();
            double frameTime = currentTime - timer;
            timer = currentTime;
            renderer.update(frameTime);

            VkResult status = swapchain.run(renderPassInfo);
            if (status == VK_SUCCESS || status == VK_SUBOPTIMAL_KHR)
            {
                renderer.encode(
                    swapchain.graphicsQueue,
                    swapchain.imageIndex,
                    swapchain.getCommandBuffer(),
                    renderPassInfo,
                    swapchain.getStartSemaphore(),
                    swapchain.getEndSemaphore(),
                    swapchain.getFence());

                status = swapchain.commit();
            }
            swapchain.step();
        }

        vkDeviceWaitIdle(swapchain.device);
    }

    void cleanup()
    {
        glfwDestroyWindow(window);
        glfwTerminate();

        swapchain.clean();
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
        glfwGetFramebufferSize(window, &width, &height);

        if (!window)
        {
            throw std::runtime_error("Unable to create GLFW window.");
        }
    }

    void initVulkan()
    {
        if (enableValidationLayers && !VulkanUtilities::checkValidationLayerSupport(validationLayers))
        {
            std::cerr << "Validation layers requested, but not available." << std::endl;
            enableValidationLayers = false;
        }

        VulkanUtilities::createInstance(instance, enableValidationLayers);

        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create window surface.");
        }
    }

    void run()
    {
        createWindow();
        initVulkan();

        swapchain.init(instance, surface, width, height);
        renderer.init(swapchain, width, height);

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
