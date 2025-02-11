#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"
#include "Component.h"

using namespace std;

GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Program> progIM; // immediate mode
shared_ptr<Shape> shape;
shared_ptr<Shape> jointShape;
shared_ptr<Component> torso;
shared_ptr<Component> rightUpperArm;
shared_ptr<Component> leftLowerArm;

Component* selectedComponent = nullptr; 
vector<Component*> componentList; 
int selectedIndex = 0;

float rotate1 = 0;
float rotate2 = 0;

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow* window, unsigned int key) {
	switch (key) {
	case '.': // Move forward in hierarchy
		selectedIndex = (selectedIndex + 1) % componentList.size();
		selectedComponent = componentList[selectedIndex];
		break;
	case ',': // Move backward in hierarchy
		selectedIndex = (selectedIndex - 1 + componentList.size()) % componentList.size();
		selectedComponent = componentList[selectedIndex];
		break;
	case 'x': // Rotate +X
		selectedComponent->jointAngles.x += 5.0f;
		break;
	case 'X': // Rotate -X
		selectedComponent->jointAngles.x -= 5.0f;
		break;
	case 'y': // Rotate +Y
		selectedComponent->jointAngles.y += 5.0f;
		break;
	case 'Y': // Rotate -Y
		selectedComponent->jointAngles.y -= 5.0f;
		break;
	case 'z': // Rotate +Z
		selectedComponent->jointAngles.z += 5.0f;
		break;
	case 'Z': // Rotate -Z
		selectedComponent->jointAngles.z -= 5.0f;
		break;
	}
}

void buildComponentList(const std::shared_ptr<Component>& root, std::vector<Component*>& list) {
	if (!root) return;
	list.push_back(root.get());
	for (const auto& child : root->children) {
		buildComponentList(child, list);
	}
}


static void init()
{
	GLSL::checkVersion();

	// Check how many texture units are supported in the vertex shader
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << endl;

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RES_DIR + "cube.obj");
	shape->init();

	jointShape = make_shared<Shape>();
	jointShape->loadMesh(RES_DIR + "sphere.obj");
	jointShape->init();
	
	// Initialize the GLSL programs.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "nor_vert.glsl", RES_DIR + "nor_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	progIM = make_shared<Program>();
	progIM->setVerbose(true);
	progIM->setShaderNames(RES_DIR + "simple_vert.glsl", RES_DIR + "simple_frag.glsl");
	progIM->init();
	progIM->addUniform("P");
	progIM->addUniform("MV");
	progIM->setVerbose(false);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);

	// Create components
	torso = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> head = std::make_shared<Component>(shape, jointShape);
	rightUpperArm = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> rightLowerArm = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> leftUpperArm = make_shared<Component>(shape, jointShape);
	leftLowerArm = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> rightUpperLeg = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> rightLowerLeg = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> leftUpperLeg = make_shared<Component>(shape, jointShape);
	shared_ptr<Component> leftLowerLeg = make_shared<Component>(shape, jointShape);

	// Set joint positions (relative to parent)
	head->jointPos = glm::vec3(0.0f, 0.45f, 0.0f); // Neck joint
	head->meshOffset = glm::vec3(0.0f, 0.2f, 0.0f); // Center of head

	rightUpperArm->jointPos = glm::vec3(0.3f, 0.4f, 0.0f); // Shoulder joint
	rightUpperArm->meshOffset = glm::vec3(0.0f, -0.2f, 0.0f); // Offset to center of arm
	rightUpperArm->jointAngles = glm::vec3(0.0f, 0.0f, 90.0f);

	rightLowerArm->jointPos = glm::vec3(0.0f, -0.4f, 0.0f); // Elbow joint
	rightLowerArm->meshOffset = glm::vec3(0.0f, -0.2f, 0.0f); // Offset to center of forearm
	rightLowerArm->jointAngles = glm::vec3(0.0f, 0.0f, 0.0f);


	leftUpperArm->jointPos = glm::vec3(-0.3f, 0.4f, 0.0f); // Shoulder joint
	leftUpperArm->meshOffset = glm::vec3(0.0f, -0.2f, 0.0f); // Offset to center of arm
	leftUpperArm->jointAngles = glm::vec3(0.0f, 0.0f, -90.0f);

	leftLowerArm->jointPos = glm::vec3(0.0f, -0.4f, 0.0f); // Elbow joint
	leftLowerArm->meshOffset = glm::vec3(0.0f, -0.2f, 0.0f); // Offset to center of forearm
	leftLowerArm->jointAngles = glm::vec3(0.0f, 0.0f, 0.0f);


	rightUpperLeg->jointPos = glm::vec3(0.2f, -0.5f, 0.0f); // Hip joint
	rightUpperLeg->meshOffset = glm::vec3(0.0f, -0.3f, 0.0f); // Offset to center of thigh
	rightUpperLeg->jointAngles = glm::vec3(0.0f, 0.0f, 0.0f);
	rightLowerLeg->jointPos = glm::vec3(0.0f, -0.6f, 0.0f); // Knee joint
	rightLowerLeg->meshOffset = glm::vec3(0.0f, -0.3f, 0.0f); // Offset to center of shin

	leftUpperLeg->jointPos = glm::vec3(-0.2f, -0.5f, 0.0f); // Hip joint
	leftUpperLeg->meshOffset = glm::vec3(0.0f, -0.3f, 0.0f); // Offset to center of thigh
	leftLowerLeg->jointPos = glm::vec3(0.0f, -0.6f, 0.0f); // Knee joint
	leftLowerLeg->meshOffset = glm::vec3(0.0f, -0.3f, 0.0f); // Offset to center of shin

	// Set scaling (optional, adjust for better proportions)
	torso->scale = glm::vec3(0.6f, 1.0f, 0.3f);
	head->scale = glm::vec3(0.3f, 0.3f, 0.3f);
	rightUpperArm->scale = leftUpperArm->scale = glm::vec3(0.2f, 0.4f, 0.2f);
	rightLowerArm->scale = leftLowerArm->scale = glm::vec3(0.15f, 0.4f, 0.15f);
	rightUpperLeg->scale = leftUpperLeg->scale = glm::vec3(0.25f, 0.6f, 0.25f);
	rightLowerLeg->scale = leftLowerLeg->scale = glm::vec3(0.2f, 0.6f, 0.2f);

	// Set up hierarchy
	torso->addChild(head);
	torso->addChild(rightUpperArm);
	rightUpperArm->addChild(rightLowerArm);
	torso->addChild(leftUpperArm);
	leftUpperArm->addChild(leftLowerArm);
	torso->addChild(rightUpperLeg);
	rightUpperLeg->addChild(rightLowerLeg);
	torso->addChild(leftUpperLeg);
	leftUpperLeg->addChild(leftLowerLeg);
	
	buildComponentList(torso, componentList);
	selectedComponent = componentList[selectedIndex];

}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));
	// Apply camera transform.
	MV->pushMatrix();
	MV->translate(glm::vec3(0, 0, -3));
	
	// Draw torso
	prog->bind();
	double t = glfwGetTime();
	MV->pushMatrix();
	//MV->translate(0.0, -0.5, 0.0);
	//MV->rotate(t, 0.0, 1.0, 0.0);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P->topMatrix()[0][0]);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
	//shape->draw(prog);

	rotate1 += 1;
	rotate2 += 5;

	torso->draw(MV, prog);

	glm::vec3 appliedScale = selectedComponent->scale;
	float A = 0.1f;
	float f = 2.0f;
	float scaleFactor = 1.0f + A/2 * A/2*(sin(2.0f * M_PI * f * t));
	appliedScale *= scaleFactor;
	selectedComponent->scale = appliedScale;

	leftLowerArm->localRot.y = rotate2;
	rightUpperArm->localRot.y = rotate1;

	MV->popMatrix();
	prog->unbind();
	


	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// https://en.wikipedia.org/wiki/OpenGL
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "AIDAN THOMAS", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);

	glfwSetCharCallback(window, char_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// Render scene.
			render();
			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
