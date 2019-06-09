#include "Object.hpp"
#include "MeshUtilities.hpp"

Object::Object(std::string &name, std::string &path)
{
    _name = name;
    _path = path;
    params.model = glm::mat4(0.0f);
}

Object::~Object()
{
}

void Object::init(
    VkPhysicalDevice &physicalDevice,
    VkDevice &device,
    VkCommandPool &commandPool,
    VkQueue &graphicsQueue)
{
    Mesh mesh;
    MeshUtilities::loadMesh(_path, mesh);
}