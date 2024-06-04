#include "instance.hpp"

#include <iostream>

bool checkExtensionSupport(std::vector<const char*> extensions) {
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

	int matches_found = 0;
	// Loop over each requested extension
	for (const char* extension_name : extensions) {
		bool extension_found = false;
		// Loop through all available extensions, flagging if a match is found
		for (VkExtensionProperties& extension : available_extensions) {
			// Does the available and requested extension match?
			if (strcmp(extension_name, extension.extensionName) == 0) {
				extension_found = true;
				matches_found++;
				break;
			}
		}
	}

	// If the number of matches equals the requested layers, then we have support
	return matches_found == extensions.size();
}

bool checkValidationLayerSupport(std::vector<const char*> validation_layers) {
	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	int matches_found = 0;
	// Loop over each requested validation layer
	for (const char* layer_name : validation_layers) {
		bool layer_found = false;
		// Loop through all available layers, flagging if a match is found
		for (VkLayerProperties& layer : available_layers) {
			// Does the available and requested layer match?
			if (strcmp(layer_name, layer.layerName) == 0) {
				layer_found = true;
				matches_found++;
				break;
			}
		}
	}

	// If the number of matches equals the requested layers, then we have support
	return matches_found == validation_layers.size();
}

VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

/////////////////
///	Callbacks ///
/////////////////
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

namespace initium {
	std::expected<std::unique_ptr<Instance>, std::string> create_instance(InstanceParams params) {
		// Are all extensions supported?
		if (!checkExtensionSupport(params.extensions)) {
			return std::unexpected("One or more requested extensions are unavailable");
		}

		// Validation layer code
		std::vector<const char*> validation_layers = {};
		if (params.enable_validation_layers) {
			// Add in layers
			validation_layers.push_back("VK_LAYER_KHRONOS_validation");

			// Are all layers supported?
			if (!checkValidationLayerSupport(validation_layers)) {
				return std::unexpected("One or more validation layers are unavailable");
			}
		}

		// Debug messenger create info
		VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = debugCallback,
			.pUserData = nullptr
		};

		// Application info structure
		VkApplicationInfo app_info{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = params.application_name,
			.applicationVersion = VK_MAKE_VERSION(std::get<0>(params.application_version), std::get<1>(params.application_version), std::get<2>(params.application_version)),
			.pEngineName = "Initium",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = VK_API_VERSION_1_3
		};
		
		// Instance create structure
		VkInstanceCreateInfo create_info{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = params.enable_validation_layers ? (VkDebugUtilsMessengerCreateInfoEXT*)&messenger_create_info : nullptr,
			.pApplicationInfo = &app_info,
			.enabledLayerCount = static_cast<uint32_t>(validation_layers.size()),
			.ppEnabledLayerNames = validation_layers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(params.extensions.size()),
			.ppEnabledExtensionNames = params.extensions.data(),
		};

		// Instance creation and validation
		VkInstance instance = VK_NULL_HANDLE;
		if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
			return std::unexpected("Instance creation failed");
		}

		// Debug messenger creation and validation
		VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
		if (params.enable_validation_layers) {
			if (createDebugUtilsMessengerEXT(instance, &messenger_create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
				return std::unexpected("Debug messenger creation failed");
			}
		}

		// Return an Instance by reference
		return std::make_unique<Instance>(instance, debug_messenger);
	}

	Instance::Instance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger) : instance_(instance), debug_messenger_(debug_messenger) {}

	Instance::~Instance() {
		if (debug_messenger_ != VK_NULL_HANDLE) {
			destroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
		}
		if (instance_ != VK_NULL_HANDLE) {
			vkDestroyInstance(instance_, nullptr);
		}
	}
}