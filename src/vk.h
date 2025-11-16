#pragma once

#include <vulkan/vulkan.h>
#include "basic.h"
#include "window_properties.h" // For surface creation parameters

#define load_instance_function( instance, name ) instance->name = (PFN_##name) vkGetInstanceProcAddr(instance->handle, #name) 
#define declare_instance_function( name ) PFN_##name name

typedef struct{
	VkInstance handle;
	VkDebugUtilsMessengerEXT debug_messenger_handle;
	declare_instance_function(vkGetPhysicalDeviceVideoFormatPropertiesKHR);
	declare_instance_function(vkGetPhysicalDeviceVideoCapabilitiesKHR);
}Instance;

extern VkAllocationCallbacks* vkb;

VkResult VK_CALL(VkResult result, b32 assert);
Instance* create_instance(Arena* arena);
void destroy_instance(Instance* instance);

typedef struct{
	Instance* instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties;
}PhysicalDeviceProperties;
PhysicalDeviceProperties pick_best_physical_device(Instance* instance, VkPhysicalDeviceType type);
PhysicalDeviceProperties get_physical_device_properties_by_index(Instance* instance, u32 physical_device_index);
PhysicalDeviceProperties get_physical_device_properties(Instance* instance, VkPhysicalDevice physical_device);

struct Device;
struct DeviceQueueFamily;
struct DeviceArena;

typedef struct DeviceQueue{
	struct DeviceQueueFamily* family;
	VkQueue handle;
	u32 index;
}DeviceQueue;

typedef struct DeviceQueueFamily{
	struct Device* device;
	DeviceQueue* queues;
	u32 queue_count;
	u32 family_index;
	// Will have at least one queue.
	// May point to the same underlying struct, there could only be one queue family, there are usuaully three.
}DeviceQueueFamily;


#define load_device_function( device, name ) device->name = (PFN_##name) vkGetDeviceProcAddr(device->handle, #name)
#define declare_device_function( name ) PFN_##name name



typedef struct Device{
	VkDevice handle;
	Instance* instance;

	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties properties;
	VkPhysicalDeviceMemoryProperties memory_properties;
	VkPhysicalDeviceConservativeRasterizationPropertiesEXT conservative_rasterization_properties;
	VkQueueFamilyProperties* queue_family_properties;

	u32 queue_family_count;
	DeviceQueueFamily* queue_families;
	DeviceQueueFamily* main_queue_family;
	DeviceQueueFamily* transfer_queue_family;
	DeviceQueueFamily* async_queue_family;



	struct DeviceArena *explicit_arena;
}Device;


Device* create_device(PhysicalDeviceProperties properties, Arena* arena);
Device* create_best_device(Instance* instance, Arena* arena);
Device* create_device_by_type(Instance* instance, VkPhysicalDeviceType type, Arena* arena);
Device* create_device_by_index(Instance* instance, u32 physical_device_index, Arena* arena);
Device** create_all_devices(Instance* instance, Arena* arena);

void destroy_device(Device* device);
void device_wait_idle(Device* device);




typedef struct{
	Instance* instance;
	VkSurfaceKHR handle;
}DeviceSurface;


DeviceSurface create_device_surface(Instance *instance, WindowDriverType driver_type, void *driver);
void destroy_device_surface(DeviceSurface surface);

typedef struct{
	Device* device;
	DeviceSurface* surface;
	VkSurfaceCapabilitiesKHR capabilities;
	uvec2 min_size;
	uvec2 max_size;
	uvec2 size;
}DeviceSurfaceInfo;

DeviceSurfaceInfo get_device_surface_info(Device* device, DeviceSurface* surface);

typedef struct{
	VkSurfaceFormatKHR surface_format;
	VkImageUsageFlags usage_flags;
}DeviceSurfaceFormat;

DeviceSurfaceFormat pick_surface_format(Device* device, DeviceSurface* surface);

typedef struct{
	VkFence handle;
	Device* device;
}DeviceFence;

typedef struct DeviceSemaphore{
	VkSemaphore handle;
	Device* device;
}DeviceSemaphore;

DeviceFence create_device_fence(Device* device, b32 signaled);
void destroy_device_fence(DeviceFence fence);
DeviceFence reset_device_fence(DeviceFence fence);
DeviceFence wait_for_device_fence(DeviceFence fence);
DeviceFence wait_for_device_fence_timeout(DeviceFence fence, u64 timeout);
void wait_and_destroy_device_fence(DeviceFence fence);
DeviceFence wait_and_reset_device_fence(DeviceFence fence);

DeviceFence* create_device_fences(Device* device, u32 fence_count, DeviceFence* fences, b32 signaled);
DeviceFence* allocate_device_fences(Device* device, u32 fence_count, b32 signaled, Arena* arena);
DeviceFence* destroy_device_fences(u32 fence_count, DeviceFence* fences);
DeviceFence* reset_device_fences(u32 fence_count, DeviceFence* fences);
DeviceFence* wait_for_device_fences(u32 fence_count, DeviceFence* fences);

DeviceSemaphore create_device_semaphore(Device* device);
void destroy_device_semaphore(DeviceSemaphore semaphore);
DeviceSemaphore recreate_device_semaphore(DeviceSemaphore* semaphore);
DeviceSemaphore* create_device_semaphores(Device* device, u32 semaphore_count, DeviceSemaphore* semaphores);
DeviceSemaphore* allocate_device_semaphores(Device* device, u32 semaphore_count, Arena* arena);
void destroy_device_semaphores(u32 semaphore_count, DeviceSemaphore* semaphores);




struct DeviceCommandBuffer;

typedef struct DeviceCommandPool{
	VkCommandPool handle;
	Device* device;
	DeviceQueueFamily* queue_family;
	Thread* parent_thread; // for debug
	VkCommandBuffer* command_buffer_handles; // array because if I dont call vkFreeCommandBuffers
}DeviceCommandPool;

typedef struct DeviceCommandBuffer{
	DeviceCommandPool* pool;
	VkCommandBuffer handle;	
}DeviceCommandBuffer;

DeviceCommandPool* allocate_device_command_pool(DeviceQueueFamily* queue_family, Arena* command_buffer_arena, Arena* arena);
DeviceCommandPool* reset_device_command_pool(DeviceCommandPool* pool, Arena* command_buffer_arena);
void free_device_command_pool(DeviceCommandPool* pool);

DeviceCommandBuffer* allocate_device_command_buffers(DeviceCommandPool* pool, b32 is_secondary, u32 count, Arena* arena);
DeviceCommandBuffer allocate_device_command_buffer(DeviceCommandPool* pool, b32 is_secondary, Arena* arena);
DeviceCommandBuffer reset_device_command_buffer(DeviceCommandBuffer cb);
void free_device_command_buffer(DeviceCommandBuffer cb);

DeviceCommandBuffer begin_device_command_buffer(DeviceCommandBuffer cb, VkCommandBufferUsageFlags usage);
DeviceCommandBuffer end_device_command_buffer(DeviceCommandBuffer cb);

void submit_device_command_buffers(DeviceQueue queue, u32 cb_count, DeviceCommandBuffer* cbs, const VkPipelineStageFlags* wait_stages, u32 wait_semaphore_count, DeviceSemaphore* wait_semaphores, u32 signal_semaphore_count, DeviceSemaphore* signal_semaphores, DeviceFence signal_fence);
void submit_device_command_buffer_blind(DeviceQueue queue, DeviceCommandBuffer cb);
void submit_device_command_buffer_simple1(DeviceQueue queue, DeviceCommandBuffer cb, DeviceFence signal_fence);
void submit_device_command_buffer_simple2(DeviceQueue queue, DeviceCommandBuffer cb, VkPipelineStageFlags wait_stage, DeviceSemaphore wait_semaphore, DeviceSemaphore signal_semaphore, DeviceFence signal_fence);

typedef struct{
	DeviceFence fence;
	DeviceCommandBuffer cb;
}TransientWait;

TransientWait submit_device_command_buffer_transient_wait(DeviceQueue queue, VkPipelineStageFlags wait_stage, DeviceCommandBuffer cb);
void transient_wait(TransientWait wait);
DeviceCommandBuffer transient_wait_preserve_cb(TransientWait wait);

typedef struct{
	VkEvent handle;
	Device* device;
}DeviceEvent;

DeviceEvent create_device_event(Device* device);
void destroy_device_event(DeviceEvent event);

void set_device_event(DeviceEvent event);
void reset_device_event(DeviceEvent event);
b32 get_event_status(DeviceEvent event);

void cmd_set_device_event(DeviceCommandBuffer cb, DeviceEvent event, VkPipelineStageFlags stage);
void cmd_reset_device_event(DeviceCommandBuffer cb, DeviceEvent event, VkPipelineStageFlags stage);

typedef struct{
	DeviceCommandPool pool;
	DeviceCommandBuffer cb;
	DeviceQueue queue;
	DeviceFence fence;
}TransientDeviceCommandBuffer;

TransientDeviceCommandBuffer begin_transient_device_command_buffer(DeviceQueue queue);
TransientDeviceCommandBuffer end_transient_device_command_buffer(TransientDeviceCommandBuffer t);
void wait_transient_device_command_buffer(TransientDeviceCommandBuffer t);


typedef struct{
	Device* device;
	VkQueryPool handle;
	u32 query_count;
	u32 was_reset;
	StringMap map;
	u64* results;
}DeviceQueryPool;

typedef struct{
	DeviceQueryPool pool;
	u32 index;
}DeviceQuery;

DeviceQueryPool create_timestamp_device_query_pool(Device* device, u32 query_count, Arena* arena);
void destroy_device_query_pool(DeviceQueryPool pool);
u32 cmd_write_timestamp(DeviceCommandBuffer cb, VkPipelineStageFlags stage, DeviceQueryPool pool, const char* name);
u32 cmd_write_timestamp_by_index(DeviceCommandBuffer cb, VkPipelineStageFlags stage, DeviceQueryPool pool, u32 index);
void cmd_reset_device_query_pool(DeviceCommandBuffer cb, DeviceQueryPool pool);
u64* get_timestamp_query_results(DeviceQueryPool pool);
u64 get_timestamp_query(DeviceQueryPool pool, const char* name);
u64 get_timestamp_query_difference(DeviceQueryPool pool, const char* a, const char* b);


typedef enum{
	DEVICE_MEMORY_TYPE_DEVICE = 0,
	DEVICE_MEMORY_TYPE_DEVICE_AND_HOST = 1,
	DEVICE_MEMORY_TYPE_HOST_COHERENT = 2,
	DEVICE_MEMORY_TYPE_HOST_CACHED = 3,
	DEVICE_MEMORY_TYPE_MAX = 4,
}DeviceMemoryTypeFlags;
typedef u32 DeviceMemoryType;

typedef struct DeviceMemoryLink{
	Device* device;
	u64 size;
	u64 pos;
	VkDeviceMemory handle;
	void* mapping;
	DeviceMemoryType type;
	struct DeviceMemoryLink* next; // link
}DeviceMemoryLink;

u32 get_memory_type_index_from_type(Device* device, DeviceMemoryType type, u32 accepted_type_bits);
DeviceMemoryLink* allocate_device_memory_link(Device* device, u64 min_size, DeviceMemoryType type, Arena* arena);
void free_device_memory_link(DeviceMemoryLink* link);

typedef struct DeviceArena{
	Device* device;
	b32 use_explicit;
	u64 total_sizes[DEVICE_MEMORY_TYPE_MAX];
	u64 min_sizes[DEVICE_MEMORY_TYPE_MAX];
	DeviceMemoryLink* links[DEVICE_MEMORY_TYPE_MAX];
	Arena* arena;
}DeviceArena;

DeviceArena* create_device_arena(Device* device, u32 min_link_size, Arena* arena);
DeviceArena* create_device_arena_explicit(Device* device, Arena* arena);
void free_device_arena(DeviceArena* device_arena);


typedef struct{
	Device* device;
	u64 size;
	void* mapping;

	u64 alignment;
	u64 offset;
	VkDeviceMemory handle;
	DeviceMemoryType type;

	DeviceMemoryLink* link;
}DeviceMemory;


DeviceMemory allocate_device_memory(DeviceArena* device_arena, u64 required_size, u64 required_alignment, DeviceMemoryType type, u32 accepted_type_bits);
DeviceMemory allocate_device_memory_explicit(Device* device, u64 size, DeviceMemoryType type, u32 accepted_type_bits);
void free_device_memory(DeviceMemory memory);

u32 get_device_allocation_count();


typedef struct{
	Device* device;
	VkBuffer handle;
	u64 size;
	u64 offset;
	DeviceMemory memory;
}DeviceBuffer;

DeviceBuffer allocate_device_buffer(DeviceArena* device_arena, u64 size, DeviceMemoryType memory_type, VkBufferUsageFlags buffer_usage, Arena* arena);
void free_device_buffer(DeviceBuffer buffer);
DeviceBuffer make_device_buffer_offset(DeviceBuffer buffer, u64 offset, u64 size);
void cmd_copy_device_buffer(DeviceCommandBuffer cb, DeviceBuffer src, DeviceBuffer dst);
void cmd_clear_device_buffer(DeviceCommandBuffer cb, const DeviceBuffer* dst);

void bind_device_buffer_memory(DeviceBuffer buffer, DeviceMemory memory);

typedef struct{
	DeviceBuffer buffer;
	VkBufferView handle; 
}DeviceBufferView;



typedef struct{
	Device* device;
	VkImage handle;
	uvec2 size;
	VkFormat format;
	VkImageTiling tiling;
	VkImageSubresourceRange subresource_range;
	DeviceMemory memory;
}DeviceImage2D;

DeviceImage2D allocate_device_image2d_long(DeviceArena* device_arena, uvec2 size, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlags sample_count, VkImageTiling tiling, Arena* arena);
DeviceImage2D allocate_device_image2d(DeviceArena* device_arena, uvec2 size, VkFormat format, VkImageUsageFlags usage, Arena* arena);
void free_device_image2d(DeviceImage2D image);

void cmd_clear_color_device_image2d(DeviceCommandBuffer cb, DeviceImage2D image, fvec4 color);
void cmd_copy_device_image2d(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst);
void cmd_blit_device_image2d(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst);
void cmd_blit_device_image2d_dst_offset(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst, uvec2 dst_offset, uvec2 dst_size);
void cmd_blit_device_image2d_no_scale(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst);
void cmd_blit_device_image2d_src_range(DeviceCommandBuffer cb, DeviceImage2D src, DeviceImage2D dst, svec2 a, svec2 b);

void cmd_transition_device_image2d(
	DeviceCommandBuffer cb, DeviceImage2D image, 
	VkImageLayout src_layout, VkImageLayout dst_layout, 
	VkAccessFlags src_access, VkAccessFlags dst_access, 
	VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage
);

void cmd_debug_barrier(DeviceCommandBuffer cb);
typedef struct{
	Device* device;
	DeviceImage2D image;
	VkImageView handle;
	uvec2 size;
	VkImageSubresourceRange subresource_range;
	VkComponentMapping component_mapping;
	// Non mutable image format
}DeviceImageView2D;

DeviceImageView2D create_device_image_view2d(DeviceImage2D image);
void destroy_device_image_view2d(DeviceImageView2D view);
void bind_device_image2d_memory(DeviceImage2D image, DeviceMemory memory);

typedef struct{
	Device* device;
	DeviceSurface surface;
	VkSwapchainKHR handle;
	u32 image_count;
	u32 image_index;
	DeviceImage2D* images;
	VkFormat format;
	uvec2 size;

	b32 should_resize;
	u64 last_resize_time;
	u64 resize_delay;

	DeviceImage2D next_image;

	VkSwapchainCreateInfoKHR previous_create_info;
}DeviceSwapchain;

DeviceSwapchain create_device_swapchain(Device* device, DeviceSurface surface, const DeviceSwapchain* old, Arena* resize_arena);
void destroy_device_swapchain(DeviceSwapchain swapchain);
void recreate_device_swapchain(DeviceSwapchain* swapchain, Arena* resize_arena);
b32 flip_device_swapchain(DeviceSwapchain* swapchain, DeviceSemaphore semaphore, DeviceFence fence);
VkResult present_device_swapchain(DeviceQueue queue, DeviceSwapchain* swapchain, DeviceSemaphore wait_semaphore);

typedef struct RenderPass{
	Device* device;
	VkRenderPass handle;	
}RenderPass;

typedef struct{
	struct RenderPass render_pass;
	uvec2 size;
	VkFramebuffer handle;
}Framebuffer;

typedef struct{
	RenderPass render_pass;
	u32 subpass_index;
}Subpass;

RenderPass create_render_pass(Device* device, VkRenderPassCreateInfo info, Arena* arena);
void destroy_render_pass(RenderPass render_pass);
Framebuffer create_framebuffer(RenderPass render_pass, uvec2 size, u32 attachment_count, DeviceImageView2D* views);
void destroy_framebuffer(Framebuffer framebuffer);



typedef struct{
	VkFormat target_color_format;
	VkSampleCountFlags sample_count;
}BasicRenderPassCreateInfo;

RenderPass* create_basic_render_pass(Device* device, BasicRenderPassCreateInfo create_info, Arena* arena);
void create_basic_render_pass_framebuffers(RenderPass* render_pass, DeviceSwapchain* swapchain, Arena* resize_arena);
void cmd_begin_render_pass_clear(DeviceCommandBuffer cb, Framebuffer fb, fvec4 clear_color);
void cmd_begin_render_pass_clear2(DeviceCommandBuffer cb, Framebuffer fb, fvec4 clear_color);
void cmd_begin_render_pass(DeviceCommandBuffer cb, Framebuffer fb);
void cmd_end_render_pass(DeviceCommandBuffer cb);


typedef struct DescriptorPool{
	Device* device;
	VkDescriptorPool handle;
}DescriptorPool;

DescriptorPool* allocate_descriptor_pool(Device* device, u32 size_count, const VkDescriptorPoolSize* sizes, Arena* arena);
void free_descriptor_pool(DescriptorPool* pool);


typedef struct{
	VkDescriptorSetLayoutBinding info;
	u32 index;
}DescriptorBindingLayout;

DescriptorBindingLayout create_descriptor_binding_layout_single(VkDescriptorType type, u32 binding, VkShaderStageFlags stage_flags, VkSampler immutable_sampler, Arena* arena);
DescriptorBindingLayout create_descriptor_binding_layout(VkDescriptorType type, u32 binding, u32 count, VkShaderStageFlags stage_flags, VkSampler* immutable_samplers, Arena* arena);
DescriptorBindingLayout create_descriptor_binding_layout_clamped_sampler(VkDescriptorType type, u32 binding, u32 count, VkShaderStageFlags stage_flags, VkSampler immutable_sampler, Arena* arena);

typedef struct{
	Device* device;
	VkDescriptorSetLayout handle;
	u32 binding_count;
	DescriptorBindingLayout* bindings;
}DescriptorSetLayout;

DescriptorSetLayout* create_descriptor_set_layout(Device* device, u32 binding_count, DescriptorBindingLayout* binding_layouts, Arena* arena);
void destroy_descriptor_set_layout(DescriptorSetLayout* layout);

struct DescriptorSet;

typedef struct{
	struct DescriptorSet* set;
	DescriptorBindingLayout layout;

}DescriptorBinding;

typedef struct DescriptorSetWriteLink{
	VkWriteDescriptorSet write;
	struct DescriptorSetWriteLink* next;
}DescriptorSetWriteLink;

typedef struct DescriptorSetCopyLink{
	VkCopyDescriptorSet copy;
	struct DescriptorSetCopyLink* next;
}DescriptorSetCopyLink;

typedef struct DescriptorSet{
	DescriptorPool* pool;
	DescriptorSetLayout* layout;
	DescriptorBinding* bindings;
	VkDescriptorSet handle;

	b32 has_changed;

	u32 write_count;
	u32 copy_count;
	DescriptorSetWriteLink* write_links;
	DescriptorSetCopyLink* copy_links;
}DescriptorSet;

DescriptorSet** allocate_descriptor_sets(DescriptorPool* pool, DescriptorSetLayout* layout, u32 count, DescriptorSet** sets, Arena* arena);
DescriptorSet* allocate_descriptor_set(DescriptorPool* pool, DescriptorSetLayout* layout, Arena* arena);
void free_descriptor_sets(u32 count, DescriptorSet** sets);
void free_descriptor_set(DescriptorSet* set);

DescriptorBinding get_descriptor_binding_from_layout_binding_layout(DescriptorBindingLayout binding_layout, DescriptorSet* set);
DescriptorBinding get_descriptor_binding_from_binding(DescriptorSet* set, u32 binding);

VkWriteDescriptorSet write_descriptor_start(DescriptorBinding descriptor);
void write_descriptor_end(DescriptorBinding binding, VkWriteDescriptorSet write, Arena* arena);

void write_buffer_descriptor(DescriptorBinding descriptor, DeviceBuffer buffer, Arena* arena);
void write_image_descriptor(DescriptorBinding descriptor, VkSampler sampler, DeviceImageView2D view, VkImageLayout layout, Arena* arena);
void update_descriptor_sets(u32 set_count, DescriptorSet** sets);

void write_image_array_descriptor(DescriptorBinding descriptor, u32 index, VkSampler sampler, DeviceImageView2D view, VkImageLayout layout, Arena* arena);





#define SHADER_PATH "src/vulkan/shaders/build/"

typedef struct{
	Device* device;
	VkPipelineLayout handle;
}PipelineLayout;

PipelineLayout create_pipeline_layout1(Device* device, DescriptorSetLayout* descriptor_set_layout, VkPushConstantRange range);
PipelineLayout create_pipeline_layout(Device* device, u32 descriptor_set_layout_count, DescriptorSetLayout** descriptor_set_layouts, u32 range_count, VkPushConstantRange* ranges);
PipelineLayout create_pipeline_layout_old(Device* device, u32 descriptor_set_layout_count, VkDescriptorSetLayout* descriptor_set_layouts, u32 range_count, VkPushConstantRange* ranges);
void destroy_pipeline_layout(PipelineLayout layout);

typedef struct{
	Device* device;
	VkShaderModule handle;
}ShaderModule;

ShaderModule create_shader_module(Device* device, u64 code_size, const u32* code);
ShaderModule create_shader_module_from_file(Device* device, const char* path, Arena* arena);
void destroy_shader_module(ShaderModule module);


typedef struct{
	Device* device;
	VkPipelineCache handle;
}PipelineCache;

PipelineCache create_pipeline_cache(Device* device);
void destroy_pipeline_cache(PipelineCache cache);

void cmd_bind_descriptor_sets(DeviceCommandBuffer cb, PipelineLayout* layout, VkPipelineBindPoint bind_point, u32 count, DescriptorSet** sets);
void cmd_bind_descriptor_set(DeviceCommandBuffer cb, PipelineLayout* layout, VkPipelineBindPoint bind_point, DescriptorSet* set);
// TODO: Dynamic offsets


typedef struct{
	Device* device;	
	VkPipeline handle;
}ComputePipeline;

ComputePipeline* create_compute_pipelines(Device* device, u32 pipeline_count, const VkComputePipelineCreateInfo* infos, ComputePipeline* pipelines);
ComputePipeline create_compute_pipeline(Device* device, VkComputePipelineCreateInfo info, Arena* arena);
ComputePipeline create_compute_pipeline_from_file(PipelineLayout layout, const char* path, PipelineCache cache, Arena* arena);

void destroy_compute_pipeline(ComputePipeline pipeline);


void cmd_bind_compute_pipeline(DeviceCommandBuffer cb, ComputePipeline pipeline);
void cmd_dispatch(DeviceCommandBuffer cb, u32 x, u32 y, u32 z);
void cmd_dispatch_square(DeviceCommandBuffer cb, uvec2 size, u32 div);



typedef struct{
	Device* device;
	VkPipeline handle;	
}GraphicsPipeline;

GraphicsPipeline* create_graphics_pipelines(Device* device, u32 pipeline_count, GraphicsPipeline* pipelines, VkGraphicsPipelineCreateInfo* infos);
GraphicsPipeline* create_graphics_pipeline(Device* device, VkGraphicsPipelineCreateInfo info, Arena* arena);

void destroy_graphics_pipeline(GraphicsPipeline pipeline);

void cmd_bind_graphics_pipeline(DeviceCommandBuffer cb, GraphicsPipeline pipeline);
void cmd_set_pipeline_size(DeviceCommandBuffer cb, uvec2 size);


