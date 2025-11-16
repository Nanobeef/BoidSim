#include "window_properties.h"

void* create_window_driver_none(WindowDriverCreateInfo info, Arena *arena)
{
	RUNTIME_ABORT("create_widow_driver_none() is EMPTY!");
	return NULL;
}

void destroy_window_driver_none(void *driver_data)
{
	RUNTIME_ABORT("destroy_widow_driver_none() is EMPTY!");
}

void poll_window_driver_none(void *driver_data, Event *event_ring_buffer)
{
	RUNTIME_ABORT("poll_widow_driver_none() is EMPTY!");
}

void* create_window_swapchain_none(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena)
{
	RUNTIME_ABORT("create_window_frameburffer_none() is EMPTY!");
	return NULL;
}

void destroy_window_swapchain_none(void *swapchain)
{
	RUNTIME_ABORT("destroy_window_frameburffer_none() is EMPTY!");
}

void present_window_swapchain_none(void *swapchain)
{
	RUNTIME_ABORT("present_window_frameburffer_none() is EMPTY!");
}

#if OS_LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h> 					
#include <dlfcn.h>			// dlopen
#endif

WindowProperties query_best_window_properties(
	WindowDriverType requested_driver,
	WindowSwapchainType requested_swapchain,
	WindowAccelerationType requested_acceleration
)
{

	b32 supported_drivers[WINDOW_DRIVER_COUNT];
	memset(supported_drivers, 0, sizeof(supported_drivers));
	b32 supported_swapchains[WINDOW_SWAPCHAIN_COUNT];
	memset(supported_swapchains, 0, sizeof(supported_swapchains));

	{
		void *handle = dlopen("libvulkan.so.1", RTLD_LAZY);
		supported_swapchains[WINDOW_SWAPCHAIN_VULKAN] = (handle != NULL);
		dlclose(handle);
	}



	#if OS_LINUX
	{
		supported_drivers[WINDOW_DRIVER_LINUX] = true;
		supported_swapchains[WINDOW_SWAPCHAIN_LINUX] = true;

		void *handle = dlopen("libX11.so.6", RTLD_LAZY);
		b32 x11_supported = (handle != NULL);
		if(handle != NULL)
		{
			dlclose(handle);
		}

		supported_drivers[WINDOW_DRIVER_X11] = x11_supported;
		supported_swapchains[WINDOW_SWAPCHAIN_X11] = x11_supported;
		supported_swapchains[WINDOW_SWAPCHAIN_X11_SHARED] = x11_supported;
	}
	#endif

	WindowDriverType chosen_driver = WINDOW_DRIVER_NONE;
	WindowSwapchainType chosen_swapchain = WINDOW_SWAPCHAIN_NONE;
	WindowAccelerationType chosen_acceleration = WINDOW_ACCELERATION_NONE;



	if(supported_drivers[requested_driver])
	{
		chosen_driver = requested_driver;
	}

	if(chosen_driver == WINDOW_DRIVER_NONE)
	{
		static const WindowDriverType driver_tier_list[] = {
			WINDOW_DRIVER_X11,
			WINDOW_DRIVER_LINUX,
		};
		for(u32 i = 0; i < arrlen(driver_tier_list); i++)
		{
			if(driver_tier_list[i])
			{
				chosen_driver = driver_tier_list[i];
				break;
			}
		}
	}

	if(supported_swapchains[WINDOW_SWAPCHAIN_VULKAN] == false)
	{
		requested_acceleration = WINDOW_ACCELERATION_SOFTWARE;
	}

	if(supported_swapchains[requested_swapchain])
	{
		chosen_swapchain = requested_swapchain;
	}
	else
	{
		if(requested_acceleration == WINDOW_ACCELERATION_HARDWARE)
		{
			if(supported_swapchains[WINDOW_SWAPCHAIN_VULKAN])
			{
				chosen_swapchain = WINDOW_SWAPCHAIN_VULKAN;
			}
		}
	}

	if(chosen_swapchain == WINDOW_SWAPCHAIN_NONE)
	{
		static const WindowDriverType swapchain_tier_list[] = {
			WINDOW_SWAPCHAIN_VULKAN,
			WINDOW_SWAPCHAIN_X11_SHARED,
			WINDOW_SWAPCHAIN_X11,
			WINDOW_SWAPCHAIN_LINUX,
		};
		for(u32 i = 0; i < arrlen(swapchain_tier_list); i++)
		{
			if(swapchain_tier_list[i])
			{
				chosen_swapchain = swapchain_tier_list[i];
				break;
			}
		}
	}
	
	if(chosen_swapchain == WINDOW_SWAPCHAIN_VULKAN)
	{
		chosen_acceleration = WINDOW_ACCELERATION_HARDWARE;
		// I guess XShm doesn't count.
	}
	else
	{
		chosen_acceleration = WINDOW_ACCELERATION_SOFTWARE;
	}

	WindowProperties ret = {
		.driver = chosen_driver,
		.swapchain = chosen_swapchain,
		.acceleration = chosen_acceleration,
	};

	memcpy(ret.supported_drivers, supported_drivers, sizeof(supported_drivers));
	memcpy(ret.supported_swapchains, supported_swapchains, sizeof(supported_swapchains));

	RUNTIME_ASSERT(ret.driver != WINDOW_DRIVER_NONE, "No window driver is supported!");
	RUNTIME_ASSERT(ret.driver != WINDOW_DRIVER_NONE, "No swapchain is supported!");

	return ret;	
}


