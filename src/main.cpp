#include "VulkanUtilities.hpp"
#include "Swapchain.hpp"
#include "Renderer.hpp"
#include "Input.hpp"

#ifdef NDEBUG
bool enableValidationLayers = false;
#else
bool enableValidationLayers = true;
#endif

// Surface params.
static const int WIDTH = 480;
static const int HEIGHT = 480;
static const int MAX_FRAMES_IN_FLIGHT = 2;

static const std::vector<const char *> validationLayers = {
    "VK_LAYER_LUNARG_standard_validation"};

static void resizeCallback(GLFWwindow *window, int width, int height)
{
    Input::instance().resizeEvent(width, height);
}

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
            Input::instance().update();
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

            if (Input::instance().isResized())
            {
                int width = 0;
                int height = 0;

                while (width == 0 || height == 0)
                {
                    glfwGetFramebufferSize(window, &width, &height);
                    glfwWaitEvents();
                }

                Input::instance().resizeEvent(width, height);

                swapchain.resize(width, height);
                renderer.resize(swapchain.renderPass, width, height);
            }

            swapchain.step();
        }

        vkDeviceWaitIdle(swapchain.device);
    }

    void cleanup()
    {
        renderer.clean();
        swapchain.clean();

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
        glfwGetFramebufferSize(window, &width, &height);

        if (!window)
        {
            throw std::runtime_error("Unable to create GLFW window.");
        }

        glfwSetFramebufferSizeCallback(window, resizeCallback);
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

        Input::instance().resizeEvent(width, height);

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
