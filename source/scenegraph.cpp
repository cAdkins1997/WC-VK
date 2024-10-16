
#include "scenegraph.h"

scenes::Node::Node(Mesh* _mesh) : mesh(_mesh){}

scenes::Node::~Node() {
    for (auto child : children) {
        ::operator delete(child);
    }
}

void scenes::Node::update_graph_transforms() {
    if (transform.isDirty) {
        if (parent)
            transform.compute_model_matrix(parent->transform.get_model_mat());
        else
            transform.compute_model_matrix();

        for (auto&& child : children) {
            child->update_graph_transforms();
        }
    }
}

void scenes::Node::update(float msec) {
    if (parent) {
        transform = parent->transform;
    }

    for (auto child : children) {
        child->update(msec);
    }
}

void scenes::Transform::compute_model_matrix() {
    modelMat = get_model_mat();
    isDirty = false;
}

void scenes::Transform::compute_model_matrix(const glm::mat4 &parentGlobalModelMatrix) {
    modelMat = parentGlobalModelMatrix * get_model_mat();
    isDirty = false;
}

void scenes::Transform::set_local_position(const glm::vec3 &newPosition) {
    pos = newPosition;
    isDirty = true;
}

glm::mat4 scenes::Transform::get_model_mat() const {
    glm::mat4 transformX = glm::rotate(
        glm::mat4{1.0f},
        glm::radians(eulerRot.x),
        glm::vec3{1.0f, 0.0f, 0.0f}
        );
    glm::mat4 transformY = glm::rotate(
        glm::mat4{1.0f},
        glm::radians(eulerRot.y),
        glm::vec3{0.0f, 1.0f, 0.0f}
    );
    glm::mat4 transformZ = glm::rotate(
        glm::mat4{1.0f},
        glm::radians(eulerRot.z),
        glm::vec3{0.0f, 0.0f, 1.0f}
    );

    glm::mat4 rotationMatrix = transformY * transformX * transformZ;

    return glm::translate(glm::mat4{1.0f}, pos) * rotationMatrix * glm::scale(glm::mat4{1.0f}, scale);
}