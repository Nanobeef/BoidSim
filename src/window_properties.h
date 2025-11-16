#pragma once
#include "event.h"
#include "image.h"

// Function pointer tables for the Window backend.

#define OS_LINUX 1
#define OS_WIN32 0
#define OS_OSX 0


typedef struct{
	s32 x,y;
	u32 width, height;
	b32 should_map_window;
	b32 should_be_fullscreen;
	void* window_pointer;
}WindowDriverCreateInfo;

typedef enum{
	WINDOW_ACCELERATION_NONE = 0,
	WINDOW_ACCELERATION_SOFTWARE = 1,
	WINDOW_ACCELERATION_HARDWARE = 2,
	WINDOW_ACCELERATION_COUNT,
}WindowAccelerationFlags;
typedef u32 WindowAccelerationType;

typedef enum{
	WINDOW_DRIVER_NONE = 0,
	WINDOW_DRIVER_LINUX = 1,
	WINDOW_DRIVER_X11 = 2,
	WINDOW_DRIVER_COUNT,
}WindowDriverTypeFlags;
typedef u32 WindowDriverType;

typedef void*	(*PFN_CreateWindowDriver)	(WindowDriverCreateInfo, Arena*);
typedef void	(*PFN_DestroyWindowDriver)	(void*);
typedef void	(*PFN_PollWindowDriver)	(void*, Event*);

void* create_window_driver_none(WindowDriverCreateInfo info, Arena *arena);
void* create_window_driver_linux(WindowDriverCreateInfo info, Arena *arena);
void* create_window_driver_x11(WindowDriverCreateInfo info, Arena *arena);

static const PFN_CreateWindowDriver create_window_driver_ppfn[WINDOW_DRIVER_COUNT] = {
	create_window_driver_none,	
	create_window_driver_linux,	
	create_window_driver_x11,	
};

void destroy_window_driver_none(void *driver_data);
void destroy_window_driver_linux(void *driver_data);
void destroy_window_driver_x11(void *driver_data);

static const PFN_DestroyWindowDriver destroy_window_driver_ppfn[WINDOW_DRIVER_COUNT] = {
	destroy_window_driver_none,	
	destroy_window_driver_linux,	
	destroy_window_driver_x11,	
};

void poll_window_driver_none(void *driver_data, Event *event_ring_buffer);
void poll_window_driver_linux(void *driver_data, Event *event_ring_buffer);
void poll_window_driver_x11(void *driver_data, Event *event_ring_buffer);

static const PFN_PollWindowDriver poll_window_driver_ppfn[WINDOW_DRIVER_COUNT] = {
	poll_window_driver_none,	
	poll_window_driver_linux,	
	poll_window_driver_x11,	
};

// SWAPCHAIN

typedef struct{
	void *old_swapchain;
	Color32 background_color;
	u32 desired_image_count;

	// Required for Vulkan swapchain
	void *device;
	WindowDriverType driver_type; // Driver is passed as parameter
}WindowSwapchainCreateInfo;

typedef enum{
	WINDOW_SWAPCHAIN_NONE = 0,
	WINDOW_SWAPCHAIN_LINUX = 1,
	WINDOW_SWAPCHAIN_X11 = 2,
	WINDOW_SWAPCHAIN_X11_SHARED = 3,
	WINDOW_SWAPCHAIN_VULKAN = 4,
	WINDOW_SWAPCHAIN_COUNT,
}WindowSwapchainTypeFlags;
typedef u32 WindowSwapchainType;

typedef void*	(*PFN_CreateWindowSwapchain)	(WindowSwapchainCreateInfo, void*, Arena*);
typedef void	(*PFN_DestroyWindowSwapchain)	(void *swapchain);
typedef void	(*PFN_PresentWindowSwapchain)	(void*);

void* create_window_swapchain_none(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena);
void* create_window_swapchain_linux(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena);
void* create_window_swapchain_x11(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena);
void* create_window_swapchain_x11_shared(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena);
void* create_window_swapchain_vulkan(WindowSwapchainCreateInfo create_info, void *driver_data, Arena *arena);

static const PFN_CreateWindowSwapchain create_window_swapchain_ppfn[WINDOW_SWAPCHAIN_COUNT] = {
	create_window_swapchain_none,	
	create_window_swapchain_linux,
	create_window_swapchain_x11,
	create_window_swapchain_x11_shared,
	create_window_swapchain_vulkan,

};

void destroy_window_swapchain_none(void *swapchain);
void destroy_window_swapchain_linux(void *swapchain);
void destroy_window_swapchain_x11(void *swapchain);
void destroy_window_swapchain_x11_shared(void *swapchain);
void destroy_window_swapchain_vulkan(void *swapchain);

static const PFN_DestroyWindowSwapchain destroy_window_swapchain_ppfn[WINDOW_SWAPCHAIN_COUNT] = {
	destroy_window_swapchain_none,	
	destroy_window_swapchain_linux,	
	destroy_window_swapchain_x11,	
	destroy_window_swapchain_x11_shared,	
	destroy_window_swapchain_vulkan,	
};


void present_window_swapchain_none(void *swapchain);
void present_window_swapchain_linux(void *swapchain);
void present_window_swapchain_x11(void *swapchain);
void present_window_swapchain_x11_shared(void *swapchain);
void present_window_swapchain_vulkan(void *swapchain);

static const PFN_PresentWindowSwapchain present_window_swapchain_ppfn[WINDOW_SWAPCHAIN_COUNT] = {
	present_window_swapchain_none,	
	present_window_swapchain_linux,	
	present_window_swapchain_x11,	
	present_window_swapchain_x11_shared,	
	present_window_swapchain_vulkan,	
};

typedef struct{
	const WindowDriverType driver;
	const WindowSwapchainType swapchain;
	const WindowAccelerationType acceleration;
	b32 supported_drivers[WINDOW_DRIVER_COUNT];
	b32 supported_swapchains[WINDOW_SWAPCHAIN_COUNT];
	void *device;
}WindowProperties;


// To use X11_SHARED as a swapchain you must request it.

WindowProperties query_best_window_properties(
	WindowDriverType requested_driver,
	WindowSwapchainType requested_swapchain,
	WindowAccelerationType requested_acceleration
);



