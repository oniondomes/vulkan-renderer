#ifndef Object_hpp
#define Object_hpp

#include "common.hpp"
#include "VulkanUtilities.hpp"

class Object
{
public:
    Object(std::string &name, const std::string &path);

    // TODO: all params could be constant
    void load(
        VkPhysicalDevice &physicalDevice,
        VkDevice &device,
        VkCommandPool &commandPool,
        VkQueue &graphicsQueue);

    void generateDescriptorSets(
        const VkDevice &device,
        const VkDescriptorPool &descriptorPool,
        const std::vector<VkBuffer> &uniformBuffers,
        VkSampler &textureSampler,
        uint32_t imageCount);

    void clean(VkDevice &device);

    static void createDescriptorSetLayout(VkDevice &_device, VkSampler &_textureSampler);
    static VkDescriptorSetLayout descriptorSetLayout;

    const VkDescriptorSet & descriptorSet(const int i){ return _descriptorSets[i]; }

    ~Object();

    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    uint32_t indicesCount;
    VulkanUtilities::ObjectInfo info;

private:
    std::string _name;
    const std::string *_path;

    VkImage _textureImage;
    VkImageView _textureImageView;

    VkDeviceMemory _vertexBufferMemory;
    VkDeviceMemory _indexBufferMemory;
    VkDeviceMemory _textureImageMemory;
    VkDeviceMemory _objectBufferMemory;

    VkBuffer _stagingBuffer;
    VkBuffer _objectBuffer;
    VkDeviceMemory _stagingBufferMemory;

    std::vector<VkDescriptorSet> _descriptorSets;
};

#endif
