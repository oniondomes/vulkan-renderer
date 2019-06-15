#ifndef Object_hpp
#define Object_hpp

#include "common.hpp"

class Object
{
public:
    Object(std::string &name, const std::string *path);

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

    static void createDescriptorSetLayout(VkDevice &_device, VkSampler &_textureSampler);
    static VkDescriptorSetLayout descriptorSetLayout;

    ~Object();

    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    uint32_t indicesCount;
    ObjectParams params;

private:
    std::string _name;
    const std::string *_path;

    VkImage _textureImage;
    VkImageView _textureImageView;

    VkDeviceMemory _vertexBufferMemory;
    VkDeviceMemory _indexBufferMemory;
    VkDeviceMemory _textureImageMemory;

    VkBuffer _stagingBuffer;
    VkDeviceMemory _stagingBufferMemory;

    std::vector<VkDescriptorSet> _descriptorSets;
};

#endif
