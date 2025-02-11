#include "Component.h"

Component::Component(glm::vec3 jointPos, glm::vec3 meshOffset, glm::vec3 scale)
    : jointPosition(jointPos), jointAngles(0.0f), meshOffset(meshOffset), scale(scale) {}

void Component::addChild(std::shared_ptr<Component> child) {
    children.push_back(child);
}

void Component::draw(std::shared_ptr<MatrixStack> stack) {
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

void Component::renderMesh() {
    // Implement rendering logic (e.g., drawing a cube, cylinder, etc.)
}
