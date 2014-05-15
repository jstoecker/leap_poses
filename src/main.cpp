#include "gl/glew.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "Renderer.h"

Renderer renderer;

GLFWwindow* initGL(int width, int height, const char* title)
{
	if (!glfwInit())
		return NULL;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		glfwTerminate();
		return NULL;
	}

	return window;
}

void resize(GLFWwindow* window, int width, int height)
{
	renderer.resize(width, height);
}

int main(int argc, char** argv)
{
	GLFWwindow* window = initGL(800, 600, "MedLeap");
	if (!window) {
		std::cout << "Couldn't initialize OpenGL" << std::endl;
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resize);
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	renderer.resize(w, h);

	while (!glfwWindowShouldClose(window)) {
		renderer.draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();

    return 0;
}

void draw()
{

}