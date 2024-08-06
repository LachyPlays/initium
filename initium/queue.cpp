#include "queue.hpp"

namespace initium {
	Queue::Queue(VkQueue queue, int queue_family) : queue_(queue), queue_family_(queue_family) {}
}