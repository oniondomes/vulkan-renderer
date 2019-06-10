#include "Object.hpp"
#include "MeshUtilities.hpp"
#include "VulkanUtilities.hpp"

Object::Object(std::string &name, const std::string *path)
{
    _name = name;
    _path = path;
    params.model = glm::mat4(0.0f);
}

Object::~Object()
{
}

void Object::load(
    VkPhysicalDevice &physicalDevice,
    VkDevice &device,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    Mesh mesh;
    MeshUtilities::loadMesh(*_path, mesh);
    indicesCount = static_cast<uint32_t>(mesh.indices.size());

    VulkanUtilities::createVertexBuffer(
        mesh,
        device,
        vertexBuffer,
        physicalDevice,
        _vertexBufferMemory,
        commandPool,
        graphicsQueue);

    VulkanUtilities::createIndexBuffer(
        mesh.indices,
        device,
        indexBuffer,
        physicalDevice,
        _indexBufferMemory,
        commandPool,
        graphicsQueue);

    // TODO: Can I cast this inline?
    std::string path = "./resources/textures/cube.png";

    VulkanUtilities::createTextureImage(
        path,
        _stagingBuffer,
        _stagingBufferMemory,
        _textureImage,
        _textureImageMemory,
        graphicsQueue,
        commandPool,
        device,
        physicalDevice);
}
