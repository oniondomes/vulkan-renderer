#ifndef Object_hpp
#define Object_hpp

#include "common.hpp"

class Object
{
public:
    Object(std::string &name, std::string &path);

    // TODO: all params could be constant
    void init(
        VkPhysicalDevice &physicalDevice,
        VkDevice &device,
        VkCommandPool &commandPool,
        VkQueue &graphicsQueue);

    ~Object();

    VkBuffer _vertexBuffer;
    VkBuffer _indexBuffer;
    uint32_t _count;
    ObjectParams params;

private:
    std::string _name;
    std::string _path;

    VkImage _textureImage;
    VkImageView _textureImageView;

    VkDeviceMemory _vertexBufferMemory;
    VkDeviceMemory _indexBufferMemory;
    VkDeviceMemory _textureImageMemory;
};

#endif
