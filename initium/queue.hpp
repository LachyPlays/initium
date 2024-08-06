#ifndef QUEUE_HPP
#define QUEUE_HPP

#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include "glfw/include/GLFW/glfw3.h"

namespace initium {
	class Queue {
	public:
		Queue(VkQueue queue, int queue_family);
	protected:
	private:
		VkQueue queue_;
		int queue_family_;
	};
}
#endif