#include "window_properties.h"


// This is for hardware copy from software rasterized buffers.

#define VK_USE_PLATFORM_XLIB_KHR
#include "vk.h"

#include "window_linux.h"

DeviceSurface create_device_surface(Instance *instance, WindowDriverType driver_type, void *driver)
{
	DeviceSurface surface = {0};
	surface.instance = instance;
	switch(driver_type)
	{
		case WINDOW_DRIVER_X11:
		{
			VkXlibSurfaceCreateInfoKHR info = {
				.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
				.dpy = ((X11WindowDriver*)driver)->display,
				.window = ((X11WindowDriver*)driver)->window,
			};
			VK_CALL(vkCreateXlibSurfaceKHR(instance->handle, &info, vkb, &surface.handle), true);
		}
		break;
		case WINDOW_DRIVER_LINUX:

		break;
		default:
		break;
	}
	return surface;
}

void* create_window_swapchain_vulkan(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena)
{
	Device *device =  create_info.device;
	DeviceSwapchain *old = create_info.old_swapchain;
	DeviceSurface surface;
	if(old == 0)
	{
		surface = create_device_surface(device->instance, create_info.driver_type, driver_data);
	}
	else
	{
		surface = old->surface;
	}
	DeviceSwapchain swapchain = create_device_swapchain(device, surface, old, arena);
	return NULL;
}

void destroy_window_swapchain_vulkan(void *swapchain)
{
	DeviceSwapchain *ws = swapchain;
	DeviceSurface surface = ws->surface;
	destroy_device_swapchain(*ws);
	destroy_device_surface(surface);
}

void present_window_swapchain_vulkan(void *swapchain)
{
	DeviceSwapchain *ws = swapchain;
}

