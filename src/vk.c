#include "vk.h"
#include "alg.h"
#include "string.h"

VkAllocationCallbacks* vkb = 0;

VkResult VK_CALL(VkResult result, b32 assert)
{
	if(result != VK_SUCCESS)
	{
		printf("VK_ASSERT(%d)\n", result);
		if(assert)
		{
			RUNTIME_ABORT("VK_CALL_FAILED!\n");
		}
	}
	return result;
}

VkBool32 debug_messanger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT* data,
	void* user_data)
{

// The following errors only happen on newer api verions. DeviceSwapchain resize stuff
	if(data->messageIdNumber == 1461184347){return VK_FALSE;}
	if(data->messageIdNumber == 1402107823){return VK_FALSE;}

	printf("%s\n\n", data->pMessage);
	return VK_FALSE;
}

Instance* create_instance(Arena* arena)
{
	const char* instance_layers[] = {
		"VK_LAYER_KHRONOS_validation",
	};
	const char* instance_extensions[] = {
		"VK_KHR_surface",
		"VK_KHR_display",
		"VK_KHR_xlib_surface",
		"VK_EXT_debug_utils",
	};

	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_0,
	};

	VkDebugUtilsMessengerCreateInfoEXT debug_messenger = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT|
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
		.messageType = 
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		.pfnUserCallback = debug_messanger_callback,
		.pUserData = 0,
	};

	VkInstanceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext = &debug_messenger,
		.pApplicationInfo = &app_info,
		.enabledLayerCount = arrlen(instance_layers),
		.ppEnabledLayerNames = instance_layers,
		.enabledExtensionCount = arrlen(instance_extensions),
		.ppEnabledExtensionNames = instance_extensions,
	};
#ifdef RELEASE
	info.enabledLayerCount = 0;
#endif


	VkInstance handle = 0;
	VkResult result = vkCreateInstance(&info, vkb, &handle);
	VK_CALL(result, true);

	VkDebugUtilsMessengerEXT debug_messenger_handle = 0;
	if(info.pNext)
	{

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugMessenger = NULL;
		vkCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(handle, "vkCreateDebugUtilsMessengerEXT");

		VK_CALL(vkCreateDebugMessenger(handle, &debug_messenger, vkb, &debug_messenger_handle), true);
	}

	Instance* instance = arena_alloc(sizeof(Instance),0,0, arena);
	*instance = (Instance){
		.handle = handle,
		.debug_messenger_handle = debug_messenger_handle,
	};

	load_instance_function(instance, vkGetPhysicalDeviceVideoFormatPropertiesKHR);
	load_instance_function(instance, vkGetPhysicalDeviceVideoCapabilitiesKHR);

	return instance;
}

void destroy_instance(Instance* instance)
{
	if(instance->debug_messenger_handle)
	{
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugMessenger = NULL;
		vkDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance->handle, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugMessenger(instance->handle, instance->debug_messenger_handle, vkb);
	}
	vkDestroyInstance(instance->handle, vkb);
}

PhysicalDeviceProperties get_physical_device_properties(Instance* instance, VkPhysicalDevice physical_device)
{
	PhysicalDeviceProperties p = {0}; 
	p.instance = instance;
	p.physical_device = physical_device;
	vkGetPhysicalDeviceProperties(p.physical_device, &p.properties);
	vkGetPhysicalDeviceMemoryProperties(p.physical_device, &p.memory_properties);
	if(0)
	{
		print("%cstr\n", p.properties.deviceName);
	}
	return p;
}

PhysicalDeviceProperties pick_best_physical_device(Instance* instance, VkPhysicalDeviceType type)
{
	Temp temp = begin_temp(0);

	u32 physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, 0);
	VkPhysicalDevice* physical_devices = arena_alloc(sizeof(VkPhysicalDevice) * physical_device_count,0,0, temp.arena);
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, physical_devices); 

	PhysicalDeviceProperties *physical_device_properties = arena_alloc(sizeof(PhysicalDeviceProperties) * physical_device_count,0,0, temp.arena);

	for(u32 i = 0; i < physical_device_count; i++)
	{
		physical_device_properties[i] = get_physical_device_properties(instance, physical_devices[i]);
	}

	u64 largest_device_local_heap = 0;
	u32 best_physical_device_index = 0;
	VkPhysicalDevice best_physical_device = physical_devices[0];

	b32 chose_pd = false;
	for(u32 i = 0; i < physical_device_count; i++)
	{
		u64 heap_size = physical_device_properties[i].memory_properties.memoryHeaps[0].size;
		b32 type_bool = ((type == physical_device_properties[i].properties.deviceType) || (type == 0));
		if((heap_size > largest_device_local_heap) && type_bool)
		{
			largest_device_local_heap = heap_size;
			best_physical_device_index = i;
		}
	}


	end_temp(temp);


	return physical_device_properties[best_physical_device_index];
}


PhysicalDeviceProperties get_physical_device_properties_by_index(Instance* instance, u32 physical_device_index)
{
	Temp temp = begin_temp(0);
	u32 physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, 0);
	VkPhysicalDevice* physical_devices = arena_alloc(sizeof(VkPhysicalDevice) * physical_device_count,0,0, temp.arena);
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, physical_devices);
	if(physical_device_index >= physical_device_count)
	{
		physical_device_index = 0;
	}
	VkPhysicalDevice physical_device = physical_devices[physical_device_index];
	PhysicalDeviceProperties properties = get_physical_device_properties(instance, physical_device);
	end_temp(temp);
	return properties;
}


Device* create_best_device(Instance* instance, Arena* arena)				// largest device-local heap size
{
	PhysicalDeviceProperties physical_device_properties = pick_best_physical_device(instance, VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
	Device* device = create_device(physical_device_properties, arena);
	return device;
}

Device* create_device_by_type(Instance* instance, VkPhysicalDeviceType type, Arena* arena)
{
	PhysicalDeviceProperties physical_device_properties = pick_best_physical_device(instance, type);
	Device* device = create_device(physical_device_properties, arena);
	return device;
}
Device* create_device_by_index(Instance* instance, u32 physical_device_index, Arena* arena)
{
	PhysicalDeviceProperties properties = get_physical_device_properties_by_index(instance, physical_device_index);
	Device* device = create_device(properties, arena);
	return device;
}

Device** create_all_devices(Instance* instance, Arena* arena)
{
	Temp temp = begin_temp(0);
	u32 physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, 0);
	VkPhysicalDevice* physical_devices = arena_alloc(physical_device_count * sizeof(VkPhysicalDevice),0,0, temp.arena);
	vkEnumeratePhysicalDevices(instance->handle, &physical_device_count, physical_devices);
	Device** devices = allocate_array(Device*, physical_device_count, arena);
	array_set_full(devices);
	for(u32 i = 0; i < physical_device_count; i++)
	{
		PhysicalDeviceProperties properties = get_physical_device_properties(instance, physical_devices[i]);
		devices[i] = create_device(properties, arena);
	}
	end_temp(temp);
	return devices;
}

b32 search_for_device_extension(const VkExtensionProperties *extensions, const char* name)
{
	for(u32 i = 0; i < array_count(extensions); i++)
	{
		if(cstrings_are_equal(extensions[i].extensionName, name))
		{
			return true;
		}
	}
	return 0;
}


Device* create_device(PhysicalDeviceProperties properties, Arena* arena)
{
	print("%cstr\n", properties.properties.deviceName);
	Temp temp = begin_temp(0);

	u32 queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(properties.physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties* queue_family_properties = arena_alloc(sizeof(VkQueueFamilyProperties) * queue_family_count,0,0, temp.arena);
	vkGetPhysicalDeviceQueueFamilyProperties(properties.physical_device, &queue_family_count, queue_family_properties);

	u32 main_queue_family_index = 0; // present
	u32 async_queue_family_index = 0; // async compute 
	u32 transfer_queue_family_index = 0; // pcie transfer

	u32 video_encode_queue_family_index = 0;
	u32 video_decode_queue_family_index = 0;
	// video encode/decode
	// rt

	{
		b32 found_main = false;
		b32 found_async = false;
		b32 found_transfer = false;
	//	b32 found_video_encode = false;
	//	b32 found_video_decode = false;
		for(u32 i = 0; i < queue_family_count; i++)
		{
			VkQueueFlags q = queue_family_properties[i].queueFlags;
			if(bitmask_contains(q, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) && !found_main)
			{
				main_queue_family_index = i;
				found_main = true;
			}
			if(bitmask_contains(q, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) &&  bitmask_does_not_contain(q, VK_QUEUE_GRAPHICS_BIT) && !found_async)
			{
				async_queue_family_index = i;
				found_async = true;
			}
			if(bitmask_contains(q, VK_QUEUE_TRANSFER_BIT) && bitmask_does_not_contain(q, VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT) && !found_transfer)
			{
				transfer_queue_family_index = i;
				found_transfer = true;
			}
			/*
			if(bitmask_contains(q, VK_QUEUE_VIDEO_ENCODE_BIT_KHR) && !found_video_encode)
			{
				video_encode_queue_family_index = i;
				found_video_decode = true;
			}
			if(bitmask_contains(q, VK_QUEUE_VIDEO_DECODE_BIT_KHR) && !found_video_decode)
			{
				video_decode_queue_family_index = i;
				found_video_encode = true;
			}
			*/
		}
	}

	// create all the queues available

	u32 queue_create_info_count = queue_family_count;
	VkDeviceQueueCreateInfo* queue_create_infos = arena_alloc(sizeof(VkDeviceQueueCreateInfo) * queue_create_info_count,0,0, temp.arena);

	
	u32 total_queue_count = 0;
	u32 total_queue_family_count = queue_family_count;
	for(u32 i = 0; i < queue_create_info_count; i++)
	{
		u32 queue_count = queue_family_properties[i].queueCount;
		total_queue_count += queue_count;
		f32* priorities = arena_alloc(queue_count * sizeof(f32), 0,0,temp.arena);
		memset_type(priorities, 1.0f, queue_count);
		queue_create_infos[i] = (VkDeviceQueueCreateInfo) {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,	
			.queueFamilyIndex = i,
			.queueCount = queue_count,
			.pQueuePriorities = priorities,
		};
	}

	
	VkPhysicalDeviceVulkan12Features enabled_features12 = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
		//.descriptorIndexing = true,
		//.shaderSampledImageArrayNonUniformIndexing = true,
	};

	VkPhysicalDeviceFeatures2 enabled_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext = &enabled_features12,
		.features.fillModeNonSolid = true,
		.features.sampleRateShading = true,
		.features.shaderFloat64 = true,
		.features.geometryShader = true,
	};

	VkExtensionProperties *extensions = 0;
	{
		u32 count = 0;
		vkEnumerateDeviceExtensionProperties(properties.physical_device, 0, &count, 0);
		extensions = allocate_array(VkExtensionProperties, count, temp.arena);
		vkEnumerateDeviceExtensionProperties(properties.physical_device, 0, &count, extensions);
		array_count(extensions) = count;
		if(0)
		{
			for(u32 i = 0; i < array_count(extensions); i++)
			{
				print("%cs\n", extensions[i].extensionName);
			}
		}
	}

	const char** extension_names = allocate_array(const char*, 16, temp.arena);
	const char* e = 0;
	append_array(extension_names, "VK_KHR_swapchain", temp.arena);


	if(0)
	{
		for(u32 i = 0; i < array_count(extension_names); i++)
		{
			print("supports: %cs\n", extension_names[i]);
		}
	}
	
	VkDeviceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &enabled_features,
		.queueCreateInfoCount = queue_create_info_count,
		.pQueueCreateInfos = queue_create_infos,
		.enabledExtensionCount = array_count(extension_names),
		.ppEnabledExtensionNames = extension_names,
	};

	VkDevice handle = 0;
	VkResult result = vkCreateDevice(properties.physical_device, &info, vkb, &handle);
	if(result == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		u32 extension_count = 0;
		vkEnumerateDeviceExtensionProperties(properties.physical_device, 0, &extension_count, NULL);
		VkExtensionProperties* extensions = arena_alloc(sizeof(VkExtensionProperties) * extension_count,0,0, temp.arena);
		vkEnumerateDeviceExtensionProperties(properties.physical_device, 0, &extension_count, extensions);
		print("EXTENSION_NOT_PRESENT: These are the available extensions:\n");
		for(u32 i = 0; i < extension_count; i++)
		{
			print("%cstr\n", extensions[i].extensionName);
		}
	}
	VK_CALL(result, true);


	Device* device = arena_alloc(sizeof(Device),0,0, arena);

	VkQueueFamilyProperties* queue_family_properties_array = allocate_array(VkQueueFamilyProperties, queue_family_count, arena);
	array_set_full(queue_family_properties_array);
	memcpy(queue_family_properties_array, queue_family_properties, sizeof(VkQueueFamilyProperties) * array_count(queue_family_properties_array));


	DeviceQueueFamily* queue_families = allocate_array(DeviceQueueFamily, total_queue_family_count, arena);
	array_set_full(queue_families);
	DeviceQueue* queues = arena_alloc(sizeof(DeviceQueue) * total_queue_count,0,0, arena);


	u32 queue_index = 0;
	for(u32 i = 0; i < queue_family_count; i++)
	{
		queue_families[i] = (DeviceQueueFamily){
			.device = device,
			.queue_count = queue_family_properties[i].queueCount,	
			.family_index = i,
			.queues = &queues[queue_index],
		};

		for(u32 j = 0; j < queue_families[i].queue_count; j++)
		{
			DeviceQueue* queue = &queue_families[i].queues[j];
			*queue = (DeviceQueue){
				.family = &queue_families[i],
				.index = j,
			};
			vkGetDeviceQueue(handle, i, j, &queue->handle);
		}
		queue_index += queue_families[i].queue_count;
	}

	*device = (Device){
		.handle = handle,	
		.instance = properties.instance,
		.physical_device = properties.physical_device,
		.properties = properties.properties,
		.memory_properties = properties.memory_properties,
		.conservative_rasterization_properties = properties.conservative_rasterization_properties,
		.queue_family_properties = queue_family_properties_array,
		.queue_families = queue_families,
		.main_queue_family = &queue_families[main_queue_family_index],
		.async_queue_family = &queue_families[async_queue_family_index],
		.transfer_queue_family = &queue_families[transfer_queue_family_index],
		.explicit_arena = create_device_arena_explicit(device, arena),
	};
	end_temp(temp);

	return device;
}

void destroy_device(Device* device)
{
	vkDestroyDevice(device->handle, vkb);	
}

void device_wait_idle(Device* device)
{
	vkDeviceWaitIdle(device->handle);
}



//typedef void (VKAPI_PTR *PFN_vkGetPhysicalDeviceFormatProperties)(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties* pFormatProperties);
DeviceSurfaceFormat pick_surface_format(Device* device, DeviceSurface* surface)
{
	Temp temp = begin_temp(0);
	// These need to match
	VkImageUsageFlags required_usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VkImageUsageFlags required_feature_flags = VK_FORMAT_FEATURE_TRANSFER_DST_BIT;

	VkSurfaceFormatKHR chosen = {0};
	u32 count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, surface->handle, &count, 0);
	VkSurfaceFormatKHR* surface_formats = arena_alloc(sizeof(VkSurfaceFormatKHR) * count,0,0, temp.arena);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, surface->handle, &count, surface_formats);

	chosen = surface_formats[0];
	for(u32 i = 0; i < count; i++)
	{
		VkFormatProperties format_properties = {0};
		vkGetPhysicalDeviceFormatProperties(device->physical_device, surface_formats[i].format, &format_properties);
		if(format_properties.optimalTilingFeatures == required_feature_flags)
		{
			chosen = surface_formats[i];
			if(chosen.format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				i = count;
			}
		}
	}

	end_temp(temp);
	DeviceSurfaceFormat ret = {
		.surface_format = chosen,
		.usage_flags = required_usage_flags,
	};
	return ret;
}

DeviceSurfaceInfo get_device_surface_info(Device* device, DeviceSurface* surface)
{
		
	VkSurfaceCapabilitiesKHR capabilities = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_device, surface->handle, &capabilities);
	DeviceSurfaceInfo info = {
		.device = device,
		.surface = surface,
		.capabilities = capabilities,
		.min_size = uvec2_make(capabilities.minImageExtent.width, capabilities.minImageExtent.height),
		.max_size= uvec2_make(capabilities.maxImageExtent.width, capabilities.maxImageExtent.height),
	};
	info.size = info.max_size;
	return info;
}


void destroy_device_surface(DeviceSurface surface)
{
	vkDestroySurfaceKHR(surface.instance->handle, surface.handle, vkb);
}



DeviceFence create_device_fence(Device* device, b32 signaled)
{
	VkFenceCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags = (signaled ? VK_FENCE_CREATE_SIGNALED_BIT : (VkFenceCreateFlags)0),
	};
	VkFence handle = 0;
	VK_CALL(vkCreateFence(device->handle, &info, vkb, &handle), true);
	DeviceFence fence = {
		.handle = handle,
		.device = device,
	};
	return fence;
}
void destroy_device_fence(DeviceFence fence)
{
	vkDestroyFence(fence.device->handle, fence.handle, vkb);
}
DeviceFence reset_device_fence(DeviceFence fence)
{
	VK_CALL(vkResetFences(fence.device->handle, 1, &fence.handle), false);
	return fence;
}
DeviceFence wait_for_device_fence(DeviceFence fence)
{
	return wait_for_device_fence_timeout(fence, U64_MAX);
}
DeviceFence wait_for_device_fence_timeout(DeviceFence fence, u64 timeout)
{
	VK_CALL(vkWaitForFences(fence.device->handle, 1, &fence.handle, true, U64_MAX), false);
	return fence;
}

DeviceFence wait_and_reset_device_fence(DeviceFence fence)
{
	wait_for_device_fence(fence);
	reset_device_fence(fence);
	return fence;
}

void wait_and_destroy_device_fence(DeviceFence fence)
{
	wait_for_device_fence(fence);
	destroy_device_fence(fence);
}


// MULTIPLE FENCES
DeviceFence* create_device_fences(Device* device, u32 fence_count, DeviceFence* fences, b32 signaled)
{
	for(u32 i = 0; i < fence_count; i++)
	{
		VkFenceCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = (signaled ? VK_FENCE_CREATE_SIGNALED_BIT : (VkFenceCreateFlags)0),
		};
		VkFence handle = 0;
		VK_CALL(vkCreateFence(device->handle, &info, vkb, &handle), true);
		fences[i] = (DeviceFence){
			.handle = handle,
			.device = device,
		};
	}
	return fences;
}
DeviceFence* allocate_device_fences(Device* device, u32 fence_count, b32 signaled, Arena* arena)
{
	DeviceFence* fences = arena_alloc(sizeof(DeviceFence) * fence_count,0,0, arena);
	return create_device_fences(device, fence_count, fences, signaled);
}
DeviceFence* destroy_device_fences(u32 fence_count, DeviceFence* fences)
{
	for(u32 i = 0; i < fence_count; i++)
	{
		vkDestroyFence(fences[i].device->handle, fences[i].handle, vkb);
	}
	return fences;
}
DeviceFence* reset_device_fences(u32 fence_count, DeviceFence* fences)
{
	Temp temp = begin_temp(0);
	VkFence* fence_handles = arena_alloc(sizeof(VkFence) * fence_count, 0,0,temp.arena);
	for(u32 i = 0; i < fence_count; i++)
	{
		fence_handles[i] = fences[i].handle;
	}
	Device* device = fences[0].device;
	VK_CALL(vkResetFences(device->handle, fence_count, fence_handles), false);
	end_temp(temp);
	return fences;
}
DeviceFence* wait_for_device_fences(u32 fence_count, DeviceFence* fences)
{
	Temp temp = begin_temp(0);
	VkFence* fence_handles = arena_alloc(sizeof(VkFence) * fence_count,0,0, temp.arena);
	for(u32 i = 0; i < fence_count; i++)
	{
		fence_handles[i] = fences[i].handle;
	}
	Device* device = fences[0].device;
	VK_CALL(vkWaitForFences(device->handle, fence_count, fence_handles, VK_TRUE, U64_MAX), false);
	end_temp(temp);
	return fences;
}

DeviceSemaphore create_device_semaphore(Device* device)
{
	VkSemaphoreCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
	};
	VkSemaphore handle = 0;
	VK_CALL(vkCreateSemaphore(device->handle, &info, vkb, &handle), true);
	DeviceSemaphore semaphore = {
		.device = device,
		.handle = handle,
	};
	return semaphore;
}
DeviceSemaphore* create_device_semaphores(Device* device, u32 semaphore_count, DeviceSemaphore* semaphores)
{
	for(u32 i = 0; i < semaphore_count; i++)
	{
		VkSemaphoreCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		VkSemaphore handle = 0;
		VK_CALL(vkCreateSemaphore(device->handle, &info, vkb, &handle), true);
		semaphores[i] = (DeviceSemaphore){
			.device = device,
			.handle = handle,
		};
	}
	return semaphores;
}
void destroy_device_semaphore(DeviceSemaphore semaphore)
{
	vkDestroySemaphore(semaphore.device->handle, semaphore.handle, vkb);
}
DeviceSemaphore recreate_device_semaphore(DeviceSemaphore* semaphore)
{
	destroy_device_semaphore(*semaphore);
	*semaphore = create_device_semaphore(semaphore->device);
	return *semaphore;
}
DeviceSemaphore* allocate_device_semaphores(Device* device, u32 semaphore_count, Arena* arena)
{
	DeviceSemaphore* semaphores = arena_alloc(sizeof(DeviceSemaphore) * semaphore_count,0,0, arena);
	return create_device_semaphores(device, semaphore_count, semaphores);
}
void destroy_device_semaphores(u32 semaphore_count, DeviceSemaphore* semaphores)
{
	for(u32 i = 0; i < semaphore_count; i++)
	{
		destroy_device_semaphore(semaphores[i]);
	}
}






DeviceEvent create_device_event(Device* device)
{
	VkEventCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO,
		.flags = 0,
	};
	VkEvent handle = 0;
	VK_CALL(vkCreateEvent(device->handle, &info, vkb, &handle), true);
	DeviceEvent event = {
		.handle = handle,
		.device = device,
	};
	return event;
}
void destroy_device_event(DeviceEvent event)
{
	vkDestroyEvent(event.device->handle, event.handle, vkb);
}

void set_device_event(DeviceEvent event)
{
	vkSetEvent(event.device->handle, event.handle);
}
void reset_device_event(DeviceEvent event)
{
	vkResetEvent(event.device->handle, event.handle);
}
b32 get_event_status(DeviceEvent event)
{
	VkResult result = vkGetEventStatus(event.device->handle, event.handle);
	if(result == VK_EVENT_SET)
	{
		return true;
	}
	else
	{
		return false;
	}
	print("Event not set or reset\n");
	return false;
}

void cmd_set_device_event(DeviceCommandBuffer cb, DeviceEvent event, VkPipelineStageFlags stage)
{
	vkCmdSetEvent(cb.handle, event.handle, stage);	
}

void cmd_reset_device_event(DeviceCommandBuffer cb, DeviceEvent event, VkPipelineStageFlags stage)
{
	vkCmdResetEvent(cb.handle, event.handle, stage);	
}










//COMMAND POOL
DeviceCommandPool* allocate_device_command_pool(DeviceQueueFamily* queue_family, Arena* command_buffer_arena, Arena* arena)
{
	VkCommandPoolCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = queue_family->family_index,
	};
	VkCommandPool handle = 0;
	VK_CALL(vkCreateCommandPool(queue_family->device->handle, &info, vkb, &handle), true);
	VkCommandBuffer* cb_handles = allocate_array(VkCommandBuffer, 16, command_buffer_arena);
	DeviceCommandPool* pool = arena_alloc(sizeof(DeviceCommandPool),0,0, arena);
	*pool = (DeviceCommandPool){
		.handle = handle,
		.device = queue_family->device,
		.parent_thread = THREAD,
		.queue_family = queue_family,
		.command_buffer_handles = cb_handles,

	};

	return pool;
}


DeviceCommandPool* reset_device_command_pool(DeviceCommandPool* pool, Arena* command_buffer_arena)
{
	u64 count = array_count(pool->command_buffer_handles);
	if(count == 0)
	{
		return pool;
	}
	VkCommandPoolResetFlags flags = VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT; 
	vkFreeCommandBuffers(pool->device->handle, pool->handle, array_count(pool->command_buffer_handles), pool->command_buffer_handles);
	vkResetCommandPool(pool->queue_family->device->handle, pool->handle, flags);
	pool->command_buffer_handles = allocate_array(VkCommandBuffer, 16, command_buffer_arena);

	return pool;
}
void free_device_command_pool(DeviceCommandPool* pool)
{
	vkDestroyCommandPool(pool->device->handle, pool->handle, vkb);	
}

// COMMAND BUFFER
DeviceCommandBuffer allocate_device_command_buffer(DeviceCommandPool* pool, b32 is_secondary, Arena* arena)
{
	VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if(is_secondary)
	{
		level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	}
	VkCommandBufferAllocateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = level,
		.commandPool = pool->handle,
		.commandBufferCount = 1,
	};
	VkCommandBuffer handle = 0;
	vkAllocateCommandBuffers(pool->device->handle, &info, &handle);
	append_array(pool->command_buffer_handles, handle, arena);
	DeviceCommandBuffer cb = {
		.pool = pool,	
		.handle = handle,
	};
	return cb;
}

DeviceCommandBuffer* allocate_device_command_buffers(DeviceCommandPool* pool, b32 is_secondary, u32 count, Arena* arena)
{
	Temp temp = begin_temp(0);
	VkCommandBuffer* handles = arena_alloc(sizeof(VkCommandBuffer) * count,0,0, temp.arena);
	{
		VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		if(is_secondary)
		{
			level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		}
		VkCommandBufferAllocateInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.level = level,
			.commandPool = pool->handle,
			.commandBufferCount = count,
		};
		vkAllocateCommandBuffers(pool->device->handle, &info, handles);
	}
	DeviceCommandBuffer* cbs = arena_alloc(sizeof(DeviceCommandBuffer) * count, 0,0,arena);
	for(u32 i = 0; i < count; i++)
	{
		cbs[i] = (DeviceCommandBuffer){
			.pool = pool,	
			.handle = handles[i],
		};
	}
	end_temp(temp);
	return cbs;
}

DeviceCommandBuffer reset_device_command_buffer(DeviceCommandBuffer cb)
{
	vkResetCommandBuffer(cb.handle, 0);
	return cb;
}

void free_device_command_buffer(DeviceCommandBuffer cb)
{
	vkFreeCommandBuffers(cb.pool->device->handle, cb.pool->handle, 1, &cb.handle);
}
DeviceCommandBuffer begin_device_command_buffer(DeviceCommandBuffer cb, VkCommandBufferUsageFlags usage)
{
	/*
    VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT 
    VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : secondary cb in render pass
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 
	*/
	VkCommandBufferBeginInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = usage,
	};
	vkBeginCommandBuffer(cb.handle, &info);
	return cb;
}
DeviceCommandBuffer end_device_command_buffer(DeviceCommandBuffer cb)
{
	vkEndCommandBuffer(cb.handle);
	return cb;
}

void submit_device_command_buffers(DeviceQueue queue, u32 cb_count, DeviceCommandBuffer* cbs, const VkPipelineStageFlags* wait_stages, u32 wait_semaphore_count, DeviceSemaphore* wait_semaphores, u32 signal_semaphore_count, DeviceSemaphore* signal_semaphores, DeviceFence signal_fence)
{
	Temp temp = begin_temp(0);
	if(wait_stages == 0)
	{
		VkPipelineStageFlags* temp_wait_stages = arena_alloc(sizeof(VkPipelineStageFlags) * cb_count,0,0, temp.arena);
		for(u32 i = 0; i < cb_count; i++)
		{
			temp_wait_stages[i] = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		}
		wait_stages = temp_wait_stages;
	}

	VkSemaphore* wait_semaphore_handles = arena_alloc(wait_semaphore_count * sizeof(VkSemaphore),0,0, temp.arena);
	for(u32 i = 0; i < wait_semaphore_count; i++)
	{
		wait_semaphore_handles[i] = wait_semaphores[i].handle;
	}

	VkSemaphore* signal_semaphore_handles = arena_alloc(signal_semaphore_count * sizeof(VkSemaphore), 0,0,temp.arena);
	for(u32 i = 0; i < signal_semaphore_count; i++)
	{
		signal_semaphore_handles[i] = signal_semaphores[i].handle;
	}
	VkCommandBuffer* cb_handles = arena_alloc(cb_count * sizeof(VkCommandBuffer),0,0, temp.arena);
	for(u32 i = 0; i < cb_count; i++)
	{
		cb_handles[i] = cbs[i].handle;
	}


	VkSubmitInfo info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pWaitDstStageMask = wait_stages,
		.waitSemaphoreCount = wait_semaphore_count,
		.pWaitSemaphores = wait_semaphore_handles,
		.signalSemaphoreCount = signal_semaphore_count,
		.pSignalSemaphores = signal_semaphore_handles,
		.commandBufferCount = cb_count,
		.pCommandBuffers = cb_handles,
	};
	VkResult result = vkQueueSubmit(queue.handle, 1, &info, signal_fence.handle);
	if(result != VK_SUCCESS)
	{
		print("Queue Submit = %s64\n", (s64)result);
	}
	end_temp(temp);
}
void submit_device_command_buffer_blind(DeviceQueue queue, DeviceCommandBuffer cb)
{
	submit_device_command_buffers(queue, 1, &cb, 0, 0,0,0,0, (DeviceFence){0});
}

void submit_device_command_buffer_simple1(DeviceQueue queue, DeviceCommandBuffer cb, DeviceFence signal_fence)
{
	submit_device_command_buffers(queue, 1, &cb, 0, 0,0,0,0, signal_fence);
}
void submit_device_command_buffer_simple2(DeviceQueue queue, DeviceCommandBuffer cb, VkPipelineStageFlags wait_stage, DeviceSemaphore wait_semaphore, DeviceSemaphore signal_semaphore, DeviceFence signal_fence)
{
	submit_device_command_buffers(queue, 1, &cb, &wait_stage, 1, &wait_semaphore, 1, &signal_semaphore, signal_fence);
}


TransientWait submit_device_command_buffer_transient_wait(DeviceQueue queue, VkPipelineStageFlags wait_stage, DeviceCommandBuffer cb)
{
	DeviceFence fence = create_device_fence(queue.family->device, false);
	submit_device_command_buffers(queue, 1, &cb, &wait_stage, 0,0, 0,0, fence);
	return (TransientWait){
		.fence = fence,	
		.cb = cb,
	};
}

void transient_wait(TransientWait wait)
{
	wait_for_device_fence(wait.fence);
	destroy_device_fence(wait.fence);
	free_device_command_buffer(wait.cb);
}

DeviceCommandBuffer transient_wait_preserve_cb(TransientWait wait)
{
	wait_for_device_fence(wait.fence);
	destroy_device_fence(wait.fence);
	return wait.cb;
}


TransientDeviceCommandBuffer begin_transient_device_command_buffer(DeviceQueue queue)
{
	DeviceCommandPool pool = {
		.device = queue.family->device,
		.queue_family = queue.family,
		.parent_thread = THREAD,
	};
	{
		VkCommandPoolCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = queue.family->family_index,
		};
		VK_CALL(vkCreateCommandPool(queue.family->device->handle, &info, vkb, &pool.handle), true);
	}

	VkCommandBuffer cb = 0;
	{
		VkCommandBufferAllocateInfo info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandPool = pool.handle,
			.commandBufferCount = 1,
		};
		vkAllocateCommandBuffers(pool.device->handle, &info, &cb);
	}
	DeviceFence fence = create_device_fence(queue.family->device, false);
	TransientDeviceCommandBuffer t = {
		.pool = pool,
		.cb.handle = cb,
		.queue = queue,
		.fence = fence,
	};
	begin_device_command_buffer(t.cb, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	return t;
}
TransientDeviceCommandBuffer end_transient_device_command_buffer(TransientDeviceCommandBuffer t)
{
	end_device_command_buffer(t.cb);
	VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	submit_device_command_buffers(t.queue, 1, &t.cb, &wait_stage, 0,0, 0,0, t.fence);
	return t;
}
void wait_transient_device_command_buffer(TransientDeviceCommandBuffer t)
{
	wait_for_device_fence(t.fence);
	destroy_device_fence(t.fence);
	free_device_command_pool(&t.pool);
}

DeviceQueryPool create_timestamp_device_query_pool(Device* device, u32 query_count, Arena* arena)
{
	VkQueryPoolCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
		.queryType = VK_QUERY_TYPE_TIMESTAMP,
		.queryCount = query_count,
	};
	VkQueryPool handle = 0;
	VK_CALL(vkCreateQueryPool(device->handle, &info, vkb, &handle), true);
	StringMap map = alloc_string_map(query_count, arena);
	u64* results = (u64*)map.pointers;
	DeviceQueryPool pool = {
		.device = device,
		.handle = handle,
		.query_count = query_count,
		.map = map,
		.results = results,
	};
	return pool;
}

void destroy_device_query_pool(DeviceQueryPool pool)
{
	vkDestroyQueryPool(pool.device->handle, pool.handle, vkb);
}

u32 cmd_write_timestamp(DeviceCommandBuffer cb, VkPipelineStageFlags stage, DeviceQueryPool pool, const char* name)
{
	u32 index = string_map_insert(pool.map, name, 0);
	vkCmdWriteTimestamp(cb.handle, stage, pool.handle, index);
	return index;
}

u32 cmd_write_timestamp_by_index(DeviceCommandBuffer cb, VkPipelineStageFlags stage, DeviceQueryPool pool, u32 index)
{
	pool.map.map[index] = "TIMESTAMP_INDEX_TAKEN";
	pool.results[index] = 0;
	vkCmdWriteTimestamp(cb.handle, stage, pool.handle, index);
	return index;
}

void cmd_reset_device_query_pool(DeviceCommandBuffer cb, DeviceQueryPool pool)
{
	memzero(pool.map.map, array_capacity_size(pool.map.map));
	vkCmdResetQueryPool(cb.handle, pool.handle, 0, pool.query_count);
}

u64* get_timestamp_query_results(DeviceQueryPool pool)
{
	vkGetQueryPoolResults(pool.device->handle, pool.handle, 0, pool.query_count, pool.query_count * sizeof(u64), pool.results, sizeof(u64), VK_QUERY_RESULT_64_BIT);
	u64 period = pool.device->properties.limits.timestampPeriod;
	if(period == 0)
	{
		print("timestamp not supported by this queue\n");	
		return pool.results;
	}
	for(u32 i = 0; i < pool.query_count; i++)
	{
		pool.results[i] = (u64)(pool.results[i] * period);
	}
	return pool.results;
}

u64 get_timestamp_query(DeviceQueryPool pool, const char* name)
{
	return (u64)string_map_lookup_raw(pool.map, name);		
}

u64 get_timestamp_query_difference(DeviceQueryPool pool, const char* a, const char* b)
{
	return get_timestamp_query(pool, b) - get_timestamp_query(pool, a);
}

u64 get_timestamp_query_by_index(DeviceQueryPool pool, u64 index)
{
	return (u64)pool.results[index];
}




const u64 min_arena_size = 16 * MB;


_Atomic u32 device_memory_allocation_count = 0;

VkMemoryPropertyFlags device_memory_type_property_flags[] = {
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
};

u32 get_memory_type_index_from_type(Device* device, DeviceMemoryType type, u32 accepted_type_bits)
{
	u32 type_bits = U32_MAX;
	if(accepted_type_bits)
	{
		type_bits = accepted_type_bits;
		//print("type_bits %b32\n", type_bits);
	}
	if((type + 1) > arrlen(device_memory_type_property_flags))
	{
		print("Memory type is not accepted, returning a compatable default!!!\n");
		type = DEVICE_MEMORY_TYPE_DEVICE_AND_HOST;
	}

	b32 has_index = 0;
	u32 index = 0;
	for(u32 itt = 0; itt < arrlen(device_memory_type_property_flags) && !has_index; itt++)
	{
		VkMemoryPropertyFlags flags = device_memory_type_property_flags[type];
		for(u32 i = 0; i < device->memory_properties.memoryTypeCount; i++)
		{
			VkMemoryPropertyFlags device_flags = device->memory_properties.memoryTypes[i].propertyFlags;

			if(((type_bits & (1<<i))) && (flags & device_flags) == flags)
			{
				has_index = true;
				index = i;
				break;
			}
		}

		switch(type)
		{
			case DEVICE_MEMORY_TYPE_DEVICE:
				type = DEVICE_MEMORY_TYPE_DEVICE_AND_HOST;
			break;
			case DEVICE_MEMORY_TYPE_DEVICE_AND_HOST:
				type = DEVICE_MEMORY_TYPE_HOST_CACHED;
			break;
			case DEVICE_MEMORY_TYPE_HOST_COHERENT:
				type = DEVICE_MEMORY_TYPE_DEVICE_AND_HOST;
			break;
			case DEVICE_MEMORY_TYPE_HOST_CACHED:
				type = DEVICE_MEMORY_TYPE_HOST_COHERENT;
			break;
			default:
				type = DEVICE_MEMORY_TYPE_DEVICE_AND_HOST;
		}
	}

	if(has_index == false)
	{
		print("Cannot find sutable device memory type index!\n");	
	}
	return index;
}

DeviceMemoryLink* allocate_device_memory_link(Device* device, u64 min_size, DeviceMemoryType type, Arena* arena)
{
	if(min_size == 0)
	{
		min_size = min_arena_size;
	}
	VkResult result = VK_ERROR_OUT_OF_DEVICE_MEMORY;
	VkDeviceMemory handle = 0;
	u64 size =  forward_align_uint(min_size, min_arena_size);
	while(result != VK_SUCCESS)
	{
		u32 type_index = get_memory_type_index_from_type(device, type, 0);
		VkMemoryAllocateInfo info = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = size,
			.memoryTypeIndex = type_index,
		};
		atomic_fetch_add(&device_memory_allocation_count, 1);

		result = vkAllocateMemory(device->handle, &info, vkb, &handle);
		if(result != VK_SUCCESS)
		{
			if(size > min_size)
			{
				size = min_size;
			}
			else
			{
				size /= 2;
			}
		}
	}
	if(handle == 0)
	{
		print("Unable to allocate device arena\n");
	}
	void* mapping = 0;
	if( type == DEVICE_MEMORY_TYPE_HOST_COHERENT || 
		type == DEVICE_MEMORY_TYPE_HOST_CACHED ||
		type == DEVICE_MEMORY_TYPE_DEVICE_AND_HOST )
	{
		vkMapMemory(
			device->handle, 
			handle, 
			0, size, 0, 
			&mapping
		);
	}
	DeviceMemoryLink* device_link = arena_alloc(sizeof(DeviceMemoryLink),0,0, arena);
	*device_link = (DeviceMemoryLink){
		.device = device,
		.size = size,
		.mapping = mapping,
		.pos = 0,
		.handle = handle,
		.type = type,
		.next = 0,
	};
	return device_link;
}

void free_device_memory_link(DeviceMemoryLink* link)
{
	if(link->size)
	{
		vkFreeMemory(link->device->handle, link->handle, vkb);
		atomic_fetch_sub(&device_memory_allocation_count, 1);
	}
	else
	{
		print("Device memory link does not have a size\n");
	}
}

DeviceArena* create_device_arena(Device* device, u32 min_link_size, Arena* arena)
{
	if(min_link_size == 0)
	{
		min_link_size = 64 * MB;
	}
	DeviceArena* device_arena = arena_alloc(sizeof(DeviceArena),0,0, arena);
	*device_arena = (DeviceArena){
		.device = device,	
		.arena = arena,
	};
	for(u32 i = 0; i < DEVICE_MEMORY_TYPE_MAX; i++)
	{
		device_arena->min_sizes[i] = min_link_size;
	}
	return device_arena;
}
void free_device_arena(DeviceArena* device_arena)
{
	for(DeviceMemoryType i = 0; i < DEVICE_MEMORY_TYPE_MAX; i++)
	{
		DeviceMemoryLink* link = device_arena->links[i];
		while(link)
		{
			free_device_memory_link(link);
			link = link->next;
		}
	}
}

DeviceArena* create_device_arena_explicit(Device* device, Arena* arena)
{
	DeviceArena* device_arena = arena_alloc(sizeof(DeviceArena),0,0, arena);
	*device_arena = (DeviceArena){
		.device = device,	
		.use_explicit = true,
		.arena = arena,
	};
	return device_arena;
}
DeviceMemory allocate_device_memory(DeviceArena* device_arena, u64 required_size, u64 required_alignment, DeviceMemoryType type, u32 accepted_type_bits)
{
	if(device_arena->use_explicit)
	{
		DeviceMemory memory = allocate_device_memory_explicit(device_arena->device, required_size, type, accepted_type_bits);
		return memory;
	}
	
	DeviceMemoryLink* dst_link = 0;
	if(device_arena->links[type] == 0)
	{
		u64 size = defmin(required_size, device_arena->min_sizes[type]);
		device_arena->links[type] = allocate_device_memory_link(device_arena->device, size, type, device_arena->arena);
		dst_link = device_arena->links[type];
	}
	else
	{
		DeviceMemoryLink* link = device_arena->links[type];
		while(link->next)
		{
			link = link->next;
		}
		u64 pos = forward_align_uint(link->pos, required_alignment);
		if(link->size - pos < required_size)
		{
			u64 size = defmax(required_size, device_arena->min_sizes[type]);
			link->next = allocate_device_memory_link(device_arena->device, size, type, device_arena->arena);
			dst_link = link->next;
		}
		else
		{
			dst_link = link;
		}
	}
	
	dst_link->pos = forward_align_uint(dst_link->pos, required_alignment);
	u64 offset = dst_link->pos;
	dst_link->pos += required_size;

	void* mapping = 0;
	if(type >= DEVICE_MEMORY_TYPE_DEVICE_AND_HOST)
	{
		mapping = (u8*)dst_link->mapping + offset;
	}

	DeviceMemory memory = {
		.device = device_arena->device,
		.mapping = mapping,
		.size = required_size,
		.offset = offset,
		.alignment = required_alignment,
		.link = dst_link,
		.handle = dst_link->handle,
		.type = type,
	};
	return memory;
}

DeviceMemory allocate_device_memory_explicit(Device* device, u64 size, DeviceMemoryType type, u32 accepted_type_bits)
{
	u32 type_index = get_memory_type_index_from_type(device, type, accepted_type_bits);
	VkMemoryAllocateInfo info = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = size,
		.memoryTypeIndex = type_index,
	};
	VkDeviceMemory handle = 0;
	VkResult result = vkAllocateMemory(device->handle, &info, vkb, &handle);
	if(result != VK_SUCCESS)
	{
		print("memory allocate fail\n");
		return (DeviceMemory){0};
	}
	atomic_fetch_add(&device_memory_allocation_count, 1);
	void* mapping = 0;
	{
		if( type == DEVICE_MEMORY_TYPE_HOST_COHERENT || 
			type == DEVICE_MEMORY_TYPE_HOST_CACHED ||
			type == DEVICE_MEMORY_TYPE_DEVICE_AND_HOST )
			vkMapMemory(
				device->handle, 
				handle, 
				0, size, 0, 
				&mapping
			);
	}

	DeviceMemory memory = {
		.device = device,
		.size = size,
		.mapping = mapping,
		.type = type,
		.handle = handle,
		.offset = 0,
		.alignment = 0,
	};
	return memory;
}

void free_device_memory(DeviceMemory memory)
{
	if(memory.link)
	{
		// will not free links from this function
	}
	else
	{
		vkFreeMemory(memory.device->handle, memory.handle, vkb);
		atomic_fetch_sub(&device_memory_allocation_count, 1);
	}
}

u32 get_device_allocation_count()
{
	return atomic_load(&device_memory_allocation_count);
}


DeviceBuffer allocate_device_buffer(DeviceArena* device_arena, u64 size, DeviceMemoryType memory_type, VkBufferUsageFlags buffer_usage, Arena* arena)
{
	VkBufferCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = buffer_usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	Device* device = device_arena->device;
	VkBuffer handle = 0;
	VK_CALL(vkCreateBuffer(device->handle, &info, vkb, &handle), true);
	VkMemoryRequirements requirements = {0};
	vkGetBufferMemoryRequirements(device->handle, handle, &requirements);

	DeviceMemory memory = allocate_device_memory(device_arena, requirements.size, requirements.alignment, memory_type, requirements.memoryTypeBits);

	DeviceBuffer buffer = {
		.device = device,
		.handle = handle,
		.size = size,
		.offset = 0,
		.memory = memory,
	};
	bind_device_buffer_memory(buffer, memory);
	return buffer;
}
void free_device_buffer(DeviceBuffer buffer)
{
	if(buffer.memory.device)
	{
		free_device_memory(buffer.memory);
		vkDestroyBuffer(buffer.device->handle, buffer.handle, vkb);
	}
}
DeviceBuffer make_device_buffer_offset(DeviceBuffer buffer, u64 offset, u64 size)
{
	buffer.offset = offset;
	buffer.size = size;
	return buffer;
}


void bind_device_buffer_memory(DeviceBuffer buffer, DeviceMemory memory)
{
	vkBindBufferMemory(buffer.device->handle, buffer.handle, memory.handle, memory.offset);
}

DeviceImage2D allocate_device_image2d_long(DeviceArena* device_arena, uvec2 size, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlags sample_count, VkImageTiling tiling, Arena* arena)
{
	VkImageCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = (VkExtent3D){size.x, size.y, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = sample_count,
		.tiling = tiling,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	Device* device = device_arena->device;
	VkImage handle = 0;
	VK_CALL(vkCreateImage(device->handle, &info, vkb, &handle), true);

	DeviceMemoryType memory_type = DEVICE_MEMORY_TYPE_DEVICE;
	if(tiling == VK_IMAGE_TILING_LINEAR)
	{
		memory_type = DEVICE_MEMORY_TYPE_HOST_CACHED;
	}
	
	VkMemoryRequirements requirements = {0};
	vkGetImageMemoryRequirements(device->handle, handle, &requirements);
	DeviceMemory memory = allocate_device_memory(device_arena, requirements.size, requirements.alignment, memory_type, requirements.memoryTypeBits);
	DeviceImage2D image = {
		.device = device,
		.handle = handle,
		.size = size,
		.tiling = info.tiling,
		.format = format,
		.subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresource_range.levelCount = 1,
		.subresource_range.layerCount = 1,
		.memory = memory,
	};
	bind_device_image2d_memory(image, memory);
	return image;
}
DeviceImage2D allocate_device_image2d(DeviceArena* device_arena, uvec2 size, VkFormat format, VkImageUsageFlags usage, Arena* arena)
{
	VkImageCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = (VkExtent3D){size.x, size.y, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};
	Device* device = device_arena->device;
	VkImage handle = 0;
	VK_CALL(vkCreateImage(device->handle, &info, vkb, &handle), true);
	
	VkMemoryRequirements requirements = {0};
	vkGetImageMemoryRequirements(device->handle, handle, &requirements);
	DeviceMemory memory = allocate_device_memory(device_arena, requirements.size, requirements.alignment, DEVICE_MEMORY_TYPE_DEVICE, requirements.memoryTypeBits);
	DeviceImage2D image = {
		.device = device,
		.handle = handle,
		.size = size,
		.format = format,
		.tiling = info.tiling,
		.subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.subresource_range.levelCount = 1,
		.subresource_range.layerCount = 1,
		.memory = memory,
	};
	bind_device_image2d_memory(image, memory);
	return image;
}
void free_device_image2d(DeviceImage2D image)
{
	if(image.memory.device)
	{
		free_device_memory(image.memory);			
		vkDestroyImage(image.device->handle, image.handle, vkb);
	}
}

void cmd_clear_color_device_image2d(DeviceCommandBuffer cb, DeviceImage2D image, fvec4 color)
{
	VkClearColorValue color_val = *(VkClearColorValue*)&color;
	vkCmdClearColorImage(cb.handle, image.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color_val, 1, &image.subresource_range);
}

void cmd_copy_device_image2d(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst)
{
	DEBUG_ASSERT(uvec2_equal(src.size, dst.size), "Vectors must be equal!");
	VkImageCopy region = {
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.srcOffset = {0,0,0},
		.dstOffset = {0,0,0},
		.extent = {src.size.x, src.size.y, 1},
	};
	vkCmdCopyImage(cb.handle, src.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}
void cmd_blit_device_image2d_dst_offset(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst, uvec2 dst_offset, uvec2 dst_size)
{
	if(dst_size.x == 0 || dst_size.y == 0)
	{
		dst_size = src.size;
	}
	VkImageBlit region = {
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.srcOffsets[0] = {0,0,0},
		.srcOffsets[1] = {src.size.x, src.size.y, 1},
		.dstOffsets[0] = {defmin(dst_offset.x, dst.size.x),defmin(dst_offset.y,dst.size.y),0},
		.dstOffsets[1] = {defmin(dst_size.x + dst_offset.x, dst.size.x), defmin(dst_size.y + dst_offset.y, dst.size.y),1},
	};
	vkCmdBlitImage(cb.handle, src.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
}
void cmd_blit_device_image2d(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst)
{
	VkImageBlit region = {
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.srcOffsets[0] = {0,0,0},
		.srcOffsets[1] = {src.size.x, src.size.y, 1},
		.dstOffsets[0] = {0,0,0},
		.dstOffsets[1] = {dst.size.x, dst.size.y,1},
	};
	vkCmdBlitImage(cb.handle, src.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
}

void cmd_blit_device_image2d_no_scale(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst)
{
	VkImageBlit region = {
		.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
		.srcOffsets[0] = {0,0,0},
		.srcOffsets[1] = {defmin(dst.size.x, src.size.x), defmin(dst.size.y, src.size.y), 1},
		.dstOffsets[0] = {0,0,0},
		.dstOffsets[1] = {dst.size.x, dst.size.y,1},
	};
	vkCmdBlitImage(cb.handle, src.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
}

void cmd_blit_device_image2d_src_range(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst, svec2 a, svec2 b)
{

	// This is very bad.
	svec2 ta = a;
	svec2 tb = b;
	a.x = defmin(ta.x, tb.x);
	a.y = defmin(ta.y, tb.y);
	b.x = defmax(ta.x, tb.x);
	b.y = defmax(ta.y, tb.y);
	{
		a.x = defmin(dst.size.x, a.x);
		a.y = defmin(dst.size.y, a.y);
		b.x = defmin(dst.size.x, b.x);
		b.y = defmin(dst.size.y, b.y);
		uvec2 src0 = uvec2_make(a.x,a.y);
		uvec2 src1 = uvec2_make(defmin(dst.size.x, b.x), defmin(dst.size.y, b.y));
		uvec2 dst0 = uvec2_make(0,0);
		uvec2 dst1 = uvec2_make(dst.size.x, dst.size.y);
		VkImageBlit region = {
			.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
			.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0,0,1},
			.srcOffsets[0] = {src0.x,src0.y,0},
			.srcOffsets[1] = {src1.x, src1.y, 1},
			.dstOffsets[0] = {dst0.x, dst0.y, 0},
			.dstOffsets[1] = {dst1.x,dst1.y,1},
		};
		vkCmdBlitImage(cb.handle, src.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst.handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_NEAREST);
	}
}
void cmd_transition_device_image2d(DeviceCommandBuffer cb, DeviceImage2D image, VkImageLayout src_layout, VkImageLayout dst_layout, VkAccessFlags src_access, VkAccessFlags dst_access, VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage)
{
	VkImageMemoryBarrier image_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,	
		.srcAccessMask = src_access,
		.dstAccessMask = dst_access,
		.oldLayout = src_layout,
		.newLayout = dst_layout,
		.image = image.handle,
		.subresourceRange = image.subresource_range,
	};
	vkCmdPipelineBarrier(cb.handle, src_stage, dst_stage, 0, 0,0, 0,0, 1,&image_barrier);
}

void cmd_debug_barrier(DeviceCommandBuffer cb)
{
	vkCmdPipelineBarrier(cb.handle, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0,0, 0,0, 0,0);
}

DeviceImageView2D create_device_image_view2d(DeviceImage2D image)
{
	VkImageViewCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image.handle,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = image.format,
		.components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
		.components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
		.subresourceRange = image.subresource_range,
	};
	VkImageView handle = 0;
	VK_CALL(vkCreateImageView(image.device->handle, &info, vkb, &handle), true);

	DeviceImageView2D view = {
		.device = image.device,
		.image = image,
		.handle = handle,
		.size = image.size,
		.subresource_range = info.subresourceRange,
		.component_mapping = info.components,
	};
	return view;
}
void destroy_device_image_view2d(DeviceImageView2D view)
{
	vkDestroyImageView(view.device->handle, view.handle, vkb);
}

void bind_device_image2d_memory(DeviceImage2D image, DeviceMemory memory)
{
	vkBindImageMemory(image.device->handle, image.handle, memory.handle, memory.offset);
}



DeviceSwapchain create_device_swapchain(Device* device, DeviceSurface surface, const DeviceSwapchain* old, Arena* resize_arena)
{
	VkSurfaceCapabilitiesKHR capabilities = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->physical_device, surface.handle, &capabilities);

	Temp temp = begin_temp(0);
	VkSurfaceFormatKHR surface_format = {0};
	{
		u32 count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, surface.handle, &count, 0);
		
		VkSurfaceFormatKHR* sfs = arena_alloc(sizeof(VkSurfaceFormatKHR) * count,0,0, temp.arena);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device->physical_device, surface.handle, &count, sfs);

		surface_format = sfs[0];
		for(u32 i = 0; i < count; i++)
		{
			if(sfs[i].format == VK_FORMAT_B8G8R8A8_SRGB)
			{
				surface_format = sfs[i];
				break;
			}
			if(sfs[i].format == VK_FORMAT_R8G8B8A8_SRGB)
			{
				surface_format = sfs[i];
				break;
			}
		}
	}

	VkSwapchainCreateInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = surface.handle,
		.minImageCount = capabilities.minImageCount,
		.imageFormat = surface_format.format,
		.imageColorSpace = surface_format.colorSpace,
		.imageExtent = capabilities.minImageExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = VK_PRESENT_MODE_FIFO_KHR,
		.clipped = VK_FALSE,
	};
	if(old)
	{
		info.oldSwapchain = old->handle;
	}

	uvec2 size = uvec2_make(info.imageExtent.width, info.imageExtent.height);

	VkSwapchainKHR handle = 0;
	VK_CALL(vkCreateSwapchainKHR(device->handle, &info, vkb, &handle), true);

	DeviceImage2D* images = 0;
	u32 image_count = 0;
	{
		vkGetSwapchainImagesKHR(device->handle, handle, &image_count, 0);
		VkImage* image_handles = arena_alloc(sizeof(VkImage) * image_count,0,0, temp.arena);
		vkGetSwapchainImagesKHR(device->handle, handle, &image_count, image_handles);

		images = arena_alloc(sizeof(DeviceImage2D) * image_count,0,0, resize_arena);

		for(u32 i = 0; i < image_count; i++)
		{
			images[i] = (DeviceImage2D){
				.device = device,
				.handle = image_handles[i],
				.format = info.imageFormat,
				.size = size,
				.subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresource_range.layerCount = info.imageArrayLayers,
				.subresource_range.levelCount = 1,
			};
		}
	}

	DeviceSwapchain swapchain = {
		.device = device,
		.surface = surface,
	 	.handle = handle,
		.image_count = image_count,
		.image_index = 0,
		.images = images,
		.format = info.imageFormat,
		.size = size,
		.resize_delay = 100000000,
		.previous_create_info = info,
	};
	end_temp(temp);
	return swapchain;
}

void destroy_device_swapchain(DeviceSwapchain swapchain)
{
	vkDestroySwapchainKHR(swapchain.device->handle, swapchain.handle, vkb);
}

void recreate_device_swapchain(DeviceSwapchain* swapchain, Arena* resize_arena)
{
	Device* device = swapchain->device;
	VkSurfaceCapabilitiesKHR capabilities = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapchain->device->physical_device, swapchain->surface.handle, &capabilities);
	swapchain->previous_create_info.imageExtent = capabilities.minImageExtent;

	VkSwapchainCreateInfoKHR info = swapchain->previous_create_info;
	info.oldSwapchain = swapchain->handle;
	VkSwapchainKHR handle = 0;
	VK_CALL(vkCreateSwapchainKHR(device->handle, &info, vkb, &handle), true);
	vkDestroySwapchainKHR(device->handle, info.oldSwapchain, vkb);
	uvec2 size = uvec2_make(info.imageExtent.width, info.imageExtent.height);

	Temp temp = begin_temp(0);
	DeviceImage2D* images = 0;
	u32 image_count = 0;
	{
		vkGetSwapchainImagesKHR(device->handle, handle, &image_count, 0);
		VkImage* image_handles = arena_alloc(sizeof(VkImage) * image_count,0,0, temp.arena);
		vkGetSwapchainImagesKHR(device->handle, handle, &image_count, image_handles);

		images = arena_alloc(sizeof(DeviceImage2D) * image_count,0,0, resize_arena);

		for(u32 i = 0; i < image_count; i++)
		{
			images[i] = (DeviceImage2D){
				.device = device,
				.handle = image_handles[i],
				.format = info.imageFormat,
				.size = size,
				.subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresource_range.layerCount = info.imageArrayLayers,
				.subresource_range.levelCount = 1,
			};
		}
	}
	swapchain->size = size;
	swapchain->handle = handle;
	swapchain->images = images;
	swapchain->image_count = image_count;
	end_temp(temp);
}

/*
b32 flip_device_swapchain(DeviceSwapchain* swapchain, DeviceSemaphore semaphore, DeviceFence fence)
{

	VkSurfaceCapabilitiesKHR capabilities = {0};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(swapchain->device->physical_device, swapchain->surface->handle, &capabilities);

	if(
	(capabilities.minImageExtent.width != swapchain->size.x) || 
	(capabilities.minImageExtent.height != swapchain->size.y)
	)
	{
		swapchain->previous_create_info.imageExtent = capabilities.minImageExtent;
		swapchain->should_recreate = true;
	}

	if(swapchain->should_recreate)
	{
		swapchain->should_recreate = false;
		return true;
	}

	VkResult result = vkAcquireNextImageKHR(
		swapchain->device->handle, 
		swapchain->handle,
		U64_MAX,
		semaphore.handle, 
		fence.handle, 
		&swapchain->image_index
	);	
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		swapchain->should_recreate = true;
	}
	swapchain->next_image = swapchain->images[swapchain->image_index];
	return false;
}
*/
VkResult present_device_swapchain(DeviceQueue queue, DeviceSwapchain* swapchain, DeviceSemaphore wait_semaphore)
{
	VkResult result;
	VkPresentInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,	
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &wait_semaphore.handle,
		.swapchainCount = 1,
		.pSwapchains = &swapchain->handle,
		.pImageIndices = &swapchain->image_index,
		.pResults = &result,
	};
	vkQueuePresentKHR(queue.handle, &info);
	if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		swapchain->should_resize= true;
	}
	return result;
}



RenderPass create_render_pass(Device* device, VkRenderPassCreateInfo info, Arena* arena)
{
	VkRenderPass handle = 0;
	VK_CALL(vkCreateRenderPass(device->handle, &info, vkb, &handle), true);
	RenderPass render_pass = (RenderPass){
		.device = device,
		.handle = handle,
	};
	return render_pass;
}
void destroy_render_pass(RenderPass render_pass)
{
	vkDestroyRenderPass(render_pass.device->handle, render_pass.handle, vkb);
}

Framebuffer create_framebuffer(RenderPass render_pass, uvec2 size, u32 attachment_count, DeviceImageView2D* views)
{
	Temp temp = begin_temp(0);
	VkImageView* attachment_handles = arena_alloc(sizeof(VkImageView) * attachment_count,0,0, temp.arena);
	for(u32 i = 0; i < attachment_count; i++)
	{
		attachment_handles[i] = views[i].handle;			
	}
	VkFramebufferCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,	
		.renderPass = render_pass.handle,
		.attachmentCount = attachment_count,
		.pAttachments = attachment_handles,
		.width = size.x,
		.height = size.y,
		.layers = 1,
	};
	VkFramebuffer handle = 0;
	VK_CALL(vkCreateFramebuffer(render_pass.device->handle, &info, vkb, &handle), true);
	Framebuffer framebuffer = {
		.render_pass = render_pass,	
		.size = size,
		.handle = handle,
	};
	end_temp(temp);
	return framebuffer;
}
void destroy_framebuffer(Framebuffer framebuffer)
{
	vkDestroyFramebuffer(framebuffer.render_pass.device->handle, framebuffer.handle, vkb);	
}

void cmd_begin_render_pass_clear(DeviceCommandBuffer cb, Framebuffer fb, fvec4 clear_color)
{
	uvec2 size = fb.size;
	VkClearValue clear_value = {.color = {{clear_color.r, clear_color.g, clear_color.b, clear_color.a}}};
	VkRenderPassBeginInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = fb.render_pass.handle,
		.framebuffer = fb.handle,
		.renderArea = (VkRect2D){{0,0},{size.x, size.y}},
		.clearValueCount = 1,
		.pClearValues = &clear_value,
	};
	vkCmdBeginRenderPass(cb.handle, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void cmd_begin_render_pass_clear2(DeviceCommandBuffer cb, Framebuffer fb, fvec4 clear_color)
{
	uvec2 size = fb.size;
	VkClearValue clear_value[2] = {
		{.color = {{clear_color.r, clear_color.g, clear_color.b, clear_color.a}}},
		{.color = {{clear_color.r, clear_color.g, clear_color.b, clear_color.a}}}
	};
	VkRenderPassBeginInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = fb.render_pass.handle,
		.framebuffer = fb.handle,
		.renderArea = (VkRect2D){{0,0},{size.x, size.y}},
		.clearValueCount = 2,
		.pClearValues = clear_value,
	};
	vkCmdBeginRenderPass(cb.handle, &info, VK_SUBPASS_CONTENTS_INLINE);
}


void cmd_begin_render_pass(DeviceCommandBuffer cb, Framebuffer fb)
{
	uvec2 size = fb.size;
	VkRenderPassBeginInfo info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = fb.render_pass.handle,
		.framebuffer = fb.handle,
		.renderArea = (VkRect2D){{0,0},{size.x, size.y}},
	};
	vkCmdBeginRenderPass(cb.handle, &info, VK_SUBPASS_CONTENTS_INLINE);
}


void cmd_end_render_pass(DeviceCommandBuffer cb)
{
	vkCmdEndRenderPass(cb.handle);
}


PipelineLayout create_pipeline_layout1(Device* device, DescriptorSetLayout* descriptor_set_layout, VkPushConstantRange range)
{
	return create_pipeline_layout(device, 1, &descriptor_set_layout, 1, &range);
}
PipelineLayout create_pipeline_layout(Device* device, u32 descriptor_set_layout_count, DescriptorSetLayout** descriptor_set_layouts, u32 range_count, VkPushConstantRange* ranges)
{
	Temp temp = begin_temp(0);

	VkDescriptorSetLayout* layout_handles = arena_alloc(sizeof(VkDescriptorSetLayout) * descriptor_set_layout_count, 0,0,temp.arena);
	for(u32 i = 0; i < descriptor_set_layout_count; i++)
	{
		layout_handles[i] = descriptor_set_layouts[i]->handle;
	}


	VkPipelineLayoutCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = descriptor_set_layout_count,
		.pSetLayouts = layout_handles,
		.pushConstantRangeCount = range_count,
		.pPushConstantRanges = ranges,
	};


	VkPipelineLayout handle = 0;
	VK_CALL(vkCreatePipelineLayout(device->handle, &info, vkb, &handle), true);
	end_temp(temp);

	PipelineLayout layout = { 
		.device = device,	
		.handle = handle,
	};
	return layout;	
}

PipelineLayout create_pipeline_layout_old(Device* device, u32 descriptor_set_layout_count, VkDescriptorSetLayout* descriptor_set_layouts, u32 range_count, VkPushConstantRange* ranges)
{
	VkPipelineLayoutCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = descriptor_set_layout_count,
		.pSetLayouts = descriptor_set_layouts,
		.pushConstantRangeCount = range_count,
		.pPushConstantRanges = ranges,
	};
	VkPipelineLayout handle = 0;
	VK_CALL(vkCreatePipelineLayout(device->handle, &info, vkb, &handle), true);

	PipelineLayout layout = { 
		.device = device,	
		.handle = handle,
	};
	return layout;
}

void destroy_pipeline_layout(PipelineLayout layout)
{
	vkDestroyPipelineLayout(layout.device->handle, layout.handle, vkb);
}

ShaderModule create_shader_module(Device* device, u64 code_size, const u32* code)
{
	VkShaderModuleCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = code_size,
		.pCode = code,
	};
	RUNTIME_ASSERT(code_size != 0, "SPV file is empty!");
	VkShaderModule handle = 0;
	VK_CALL(vkCreateShaderModule(device->handle, &info, vkb, &handle), true);
	ShaderModule module = {
		.device = device,
		.handle = handle,
	};
	return module;
}

ShaderModule create_shader_module_from_file(Device* device, const char* path, Arena* arena)
{
	Temp temp = begin_temp(arena);
	u8* file = read_entire_binary_file(path, temp.arena);
	ShaderModule module = create_shader_module(device, buffer_size(file), (const u32*)file);
	end_temp(temp);
	return module;
}

void destroy_shader_module(ShaderModule module)
{
	vkDestroyShaderModule(module.device->handle, module.handle, vkb);
}

PipelineCache create_pipeline_cache(Device* device)
{
	VkPipelineCacheCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.initialDataSize = 0,
		.pInitialData = 0,
	};
	VkPipelineCache handle = 0;
	VK_CALL(vkCreatePipelineCache(device->handle, &info, vkb, &handle), true);
	PipelineCache cache = {
		.device = device,
		.handle = handle,
	};
	return cache;
}
void destroy_pipeline_cache(PipelineCache cache)
{
	vkDestroyPipelineCache(cache.device->handle, cache.handle, vkb);
}



void cmd_bind_descriptor_sets(DeviceCommandBuffer cb, PipelineLayout* layout, VkPipelineBindPoint bind_point, u32 count, DescriptorSet** sets)
{
	Temp temp = begin_temp(0);
	VkDescriptorSet* set_handles = arena_alloc(sizeof(VkDescriptorSet) * count, 0,0,temp.arena);
	for(u32 i = 0; i < count; i++)
	{
		set_handles[i] = sets[i]->handle;	
	}
	vkCmdBindDescriptorSets(cb.handle, bind_point, layout->handle, 0, count, set_handles, 0,0);
	end_temp(temp);
}

void cmd_bind_descriptor_set(DeviceCommandBuffer cb, PipelineLayout* layout, VkPipelineBindPoint bind_point, DescriptorSet* set)
{
	cmd_bind_descriptor_sets(cb, layout, bind_point, 1, &set);
}


ComputePipeline* create_compute_pipelines(Device* device, u32 pipeline_count, const VkComputePipelineCreateInfo* infos, ComputePipeline* pipelines)
{
	Arena* temp = get_arena(0);
	u8* temp_mark = temp->pos;
	VkPipeline* handles = arena_alloc(sizeof(VkPipeline) * pipeline_count, 0,0,temp);
	VK_CALL(vkCreateComputePipelines(device->handle, 0, pipeline_count, infos,vkb, handles), true);
	for(u32 i = 0; i < pipeline_count; i++)
	{
		pipelines[i] = (ComputePipeline){
			.device = device,
			.handle = handles[i],
		};
	}
	temp->pos = temp_mark;
	return pipelines;
}

ComputePipeline create_compute_pipeline(Device* device, VkComputePipelineCreateInfo info, Arena* arena)
{
	ComputePipeline pipeline = {0};
	create_compute_pipelines(device, 1, &info, &pipeline);
	return pipeline;
}

ComputePipeline create_compute_pipeline_from_file(PipelineLayout layout, const char* path, PipelineCache cache, Arena* arena)
{
	Device* device = layout.device;
	ShaderModule module = create_shader_module_from_file(device, path, arena);

	VkPipelineShaderStageCreateInfo stage = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,	
		.stage = VK_SHADER_STAGE_COMPUTE_BIT,
		.module = module.handle,
		.pName = "main",
	};

	VkComputePipelineCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = stage,
		.layout = layout.handle,
	};

	ComputePipeline pipeline = create_compute_pipeline(device, info, arena);
	destroy_shader_module(module);
	return pipeline;
}


void destroy_compute_pipeline(ComputePipeline pipeline)
{
	vkDestroyPipeline(pipeline.device->handle, pipeline.handle, vkb);
}

void cmd_bind_compute_pipeline(DeviceCommandBuffer cb, ComputePipeline pipeline)
{
	vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.handle);
}
void cmd_dispatch(DeviceCommandBuffer cb, u32 x, u32 y, u32 z)
{
	vkCmdDispatch(cb.handle, x,y,z);
}

void cmd_dispatch_square(DeviceCommandBuffer cb, uvec2 size, u32 div)
{
	vkCmdDispatch(cb.handle, size.x / div, size.y / div, 1);
}

GraphicsPipeline* create_graphics_pipelines(Device* device, u32 pipeline_count, GraphicsPipeline* pipelines, VkGraphicsPipelineCreateInfo* infos)
{
	Temp temp = begin_temp(0);
	
	VkPipeline* handles = arena_alloc(pipeline_count * sizeof(VkPipeline),0,0, temp.arena);
	VK_CALL(vkCreateGraphicsPipelines(device->handle, 0, pipeline_count, infos, vkb, handles), true);


	for(u32 i = 0; i < pipeline_count; i++)
	{
		pipelines[i] = (GraphicsPipeline){
			.device = device,
			.handle = handles[i],
		};
	}
	end_temp(temp);
	
	return pipelines;
}
GraphicsPipeline* create_graphics_pipeline(Device* device, VkGraphicsPipelineCreateInfo info, Arena* arena)
{
	GraphicsPipeline* pipeline = arena_alloc(sizeof(GraphicsPipeline),0,0, arena);
	*pipeline = (GraphicsPipeline){
		.device = device,
	};
	VK_CALL(vkCreateGraphicsPipelines(device->handle, 0, 1, &info, vkb, &pipeline->handle), true);
	return pipeline;
}

void destroy_graphics_pipeline(GraphicsPipeline pipeline)
{
	vkDestroyPipeline(pipeline.device->handle, pipeline.handle, vkb);
}

void cmd_bind_graphics_pipeline(DeviceCommandBuffer cb, GraphicsPipeline pipeline)
{
	vkCmdBindPipeline(cb.handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle);
}

void cmd_set_pipeline_size(DeviceCommandBuffer cb, uvec2 size)
{
	VkViewport viewport = {
		.x = 0,
		.y = 0,
		.width = (f32)size.x,
		.height = (f32)size.y,
		.minDepth = 0.0,
		.maxDepth = 1.0,
	};
	VkRect2D scissor = {
		.offset = (VkOffset2D){0,0},
		.extent = (VkExtent2D){size.x, size.y},
	};
	vkCmdSetScissor(cb.handle, 0, 1, &scissor);
	vkCmdSetViewport(cb.handle, 0, 1, &viewport);

}

