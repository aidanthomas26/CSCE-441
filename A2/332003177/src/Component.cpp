#define _USE_MATH_DEFINES
#include <cmath>
#include "Component.h"
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GLSL.h"
#include "Program.h"

Component* Component::selectedComponent = nullptr;

Component::Component(std::shared_ptr<Shape> shape, std::shared_ptr<Shape> jointShape) : shape(shape), jointShape(jointShape) {
    jointPos = glm::vec3(0.0f);
    jointAngles = glm::vec3(0.0f);
    localRot = glm::vec3(0.0f);
    meshOffset = glm::vec3(0.0f);
    scale = glm::vec3(1.0f);
}

void Component::addChild(std::shared_ptr<Component> child) {
    children.push_back(child);
}

void Component::draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program>& prog) {
    //if (!shape) return;

    MV->pushMatrix();

    MV->translate(jointPos);

    MV->rotate(glm::radians(jointAngles.x), glm::vec3(1, 0, 0));
    MV->rotate(glm::radians(jointAngles.y), glm::vec3(0, 1, 0));
    MV->rotate(glm::radians(jointAngles.z), glm::vec3(0, 0, 1));

    if (jointShape) {
        MV->pushMatrix();
        MV->scale(glm::vec3(0.13f)); 
        glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
        jointShape->draw(prog);
        MV->popMatrix();
    }

    MV->pushMatrix();

    MV->translate(meshOffset);
    MV->scale(scale);
    MV->rotate(glm::radians(localRot.x), glm::vec3(1, 0, 0));
    MV->rotate(glm::radians(localRot.y), glm::vec3(0, 1, 0));
    MV->rotate(glm::radians(localRot.z), glm::vec3(0, 0, 1));

    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);

    shape->draw(prog);

    MV->popMatrix();

    for (auto& child : children) {
        child->draw(MV, prog);
    }

    MV->popMatrix();

}
