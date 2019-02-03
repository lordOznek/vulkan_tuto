#pragma once
#include "../device.h"

#include <vulkan/vulkan.h>
#include <vector>

struct GLFWwindow;
class DeviceQueueVulkan;

class SwapChainVulkan
{
public:
	~SwapChainVulkan();

public:
	VkSurfaceFormatKHR surfaceFormat = {};
	VkPresentModeKHR presentMode = {};
	VkExtent2D extent = {};
	VkSwapchainKHR m_native = {};
	uint32_t imageCount = 0U;
	VkImage * m_image = nullptr;
};

class Device3DVulkan : public IDevice3D
{
public:
	virtual void Init() override;
	virtual void Destroy() override;

public:
	void CreateInstance();
	void DestroyInstance();
	void CreateWindow();
	void DestroyWindow();
	void CreateDeviceAndQueues();
	void DestroyDeviceAndQueues();
	void CreateSwapChain();

private:
	bool CheckInstanceExtensions( const std::vector<const char *> & requiredExt );
	bool CheckDeviceExtensionSupport( VkPhysicalDevice device, const std::vector<const char *> & deviceExtensions );

private:
	VkDevice m_device = VK_NULL_HANDLE;
	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	SwapChainVulkan m_swapChain;
	GLFWwindow * m_window = nullptr;
	DeviceQueueVulkan * m_queues[QueueCount];
	DeviceQueueVulkan * & m_gfxQueue = m_queues[GraphicsQueue];
	DeviceQueueVulkan * & m_computeQueue = m_queues[ComputeQueue];
	DeviceQueueVulkan * & m_copyQueue = m_queues[CopyQueue];
	DeviceQueueVulkan * & m_presentQueue = m_queues[PresentQueue];
};