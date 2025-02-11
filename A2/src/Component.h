#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "MatrixStack.h"
#include "Shape.h"

class Component {
    public:
        glm::vec3 jointPos;   // Translation of the component's joint w.r.t. parent
        glm::vec3 jointAngles;     // Rotation angles (X, Y, Z) for the joint
        glm::vec3 localRot;
        glm::vec3 meshOffset;      // Translation of the mesh w.r.t. joint
        glm::vec3 scale;           // Scaling factors (X, Y, Z)
        std::vector<std::shared_ptr<Component>> children;
        std::shared_ptr<Shape> shape;
        std::shared_ptr<Shape> jointShape;
        static Component* selectedComponent;

        Component() : shape(nullptr), jointPos(0.0f), jointAngles(0.0f), meshOffset(0.0f), scale(1.0f) {}

        Component(std::shared_ptr<Shape> shape, std::shared_ptr<Shape> jointShape);

        Component(glm::vec3 jointPos, glm::vec3 meshOffset, glm::vec3 scale, const std::shared_ptr<Shape>& shape);

        void addChild(std::shared_ptr<Component> child);

        void draw(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program>& prog);
};