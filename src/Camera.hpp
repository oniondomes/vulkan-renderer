#ifndef Camera_hpp
#define Camera_hpp

#include "common.hpp"

class Camera
{
public:
    Camera(const float ratio = 1.0f);
    ~Camera(){};

    void setProjection(float ratio, float fov, float near, float far);
    void updatePosition(double time);
    void update();
    void updateWithKeys();
    void updateWithMouse();

    glm::mat4 getViewProjectionMatrix() const;

private:
    void _updateProjection();
    void _updateView();

    glm::mat4 _view;
    glm::mat4 _proj;

    glm::vec3 _position;
    glm::vec3 _up;
    glm::vec3 _front;

    float _fov;
    float _ratio;
    float _near;
    float _far;
    float _rotationSpeed = 0.1f;
    float _movementSpeed = 0.1f;
};

#endif
