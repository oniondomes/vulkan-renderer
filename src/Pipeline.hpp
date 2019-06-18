#ifndef Pipeline_hpp
#define Pipeline_hpp

#include "common.hpp"
#include "VulkanUtilities.hpp"
#include "MeshUtilities.hpp"
#include "Swapchain.hpp"

class Pipeline
{
public:
    Pipeline();
    ~Pipeline();

    static VkShaderModule createShaderModule(VkDevice &device, const std::vector<char> &code);
    static void create(
        VkDevice &device,
        const uint32_t width,
        const uint32_t height,
        VkDescriptorSetLayout &descriptorSetLayout,
        VkPipelineLayout &pipelineLayout,
        VkRenderPass &renderPass,
        VkPipeline &pipeline);
};

#endif
