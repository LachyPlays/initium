#ifndef INSTANCE_HPP
#define INSTANCE_HPP

#include <vulkan/vulkan.h>
#include <expected>
#include <string>
#include <tuple>
#include <vector>
#include <memory>

namespace initium {
	struct InstanceParams {
		const char* application_name = "Initium";
		std::tuple<int, int, int> application_version = { 0, 0, 0 };
		std::vector<const char*> extensions = {};
		bool enable_validation_layers = false;
	};

	class Instance {
	public:
		Instance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger);
		~Instance();

		friend std::expected<std::unique_ptr<Instance>, std::string> create_instance(InstanceParams params);
	private:
		VkInstance instance_ = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
	};

	std::expected<std::unique_ptr<Instance>, std::string> create_instance(InstanceParams params);
}
#endif