#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MatrixStack.h"

class Component {
public:
    glm::vec3 jointPosition;   // Translation of the component's joint w.r.t. parent
    glm::vec3 jointAngles;     // Rotation angles (X, Y, Z) for the joint
    glm::vec3 meshOffset;      // Translation of the mesh w.r.t. joint
    glm::vec3 scale;           // Scaling factors (X, Y, Z)
    std::vector<std::shared_ptr<Component>> children;

    Component(glm::vec3 jointPos, glm::vec3 meshOffset, glm::vec3 scale)
        : jointPosition(jointPos), jointAngles(0.0f), meshOffset(meshOffset), scale(scale) {}

    void addChild(std::shared_ptr<Component> child) {
        children.push_back(child);
    }

    void draw(std::shared_ptr<MatrixStack> stack) {
        stack->pushMatrix();
        stack->translate(jointPosition);
        stack->rotate(jointAngles.x, glm::vec3(1, 0, 0));
        stack->rotate(jointAngles.y, glm::vec3(0, 1, 0));
        stack->rotate(jointAngles.z, glm::vec3(0, 0, 1));
        stack->translate(meshOffset);
        stack->scale(scale);

        renderMesh();

        for (auto& child : children) {
            child->draw(stack);
        }

        stack->popMatrix();
    }

private:
    void renderMesh() {
        // Implement rendering logic (e.g., draw a cube, cylinder, etc.)
    }
};

