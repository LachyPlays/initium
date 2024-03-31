#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

int main() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(600, 400, "Initium", NULL, NULL);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}