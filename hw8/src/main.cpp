#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <iostream>
#include <cmath>

#include <imgui.h>
#include <sources/imgui_impl_glfw.h>
#include <sources/imgui_impl_opengl3.h>


#include "ShadowTest.h"
#include "BezierCurve.h"
using std::cout;
using std::endl;
//
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;


double xPos = 0.0f;
double yPos = 0.0f;


void framebuffer_size_callback(GLFWwindow *window, int width, int height);	//���ڴ�С�ı�
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);


BezierCurve* bezierCurve = NULL;


int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



	// create window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);


	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return 0;
	}

	const char *glsl_version = "#version 330";


	bezierCurve = new BezierCurve();
	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		bezierCurve->draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	delete bezierCurve;
	glfwTerminate();
	return 0;
}

// framebuffer size change
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	xPos = xpos / (SCR_WIDTH - 1) * 2 - 1;
	yPos = ypos / (SCR_HEIGHT - 1) * 2 - 1;
	yPos *= -1;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			bezierCurve->addControlPoint(xPos, yPos);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT:
			bezierCurve->removeControlPoint();
			break;
		}
	}
}

