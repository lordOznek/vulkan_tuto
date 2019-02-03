#include <stdafx.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Device3D_vulkan.h"
#include "DeviceQueue_vulkan.h"

#define SAFE_DELETE( ptr ) do { if ( ptr ) { delete ptr; ptr = nullptr; } } while(0)
#define SAFE_DELETE_ARRAY( ptr ) do { if ( ptr ) { delete[] ptr; ptr = nullptr; } } while(0)

namespace
{
	const int WIDTH = 800;
	const int HEIGHT = 600;
	constexpr unsigned int NUM_BACK_BUFFER = 2;
	const VkPresentModeKHR preferredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	const char * deviceExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	const uint32_t deviceExtensionCount = sizeof( deviceExtensions ) / sizeof( *deviceExtensions );
}

//
//
//	SwapChainVulkan ===============================================================================
//
//
SwapChainVulkan::~SwapChainVulkan()
{
	SAFE_DELETE_ARRAY( m_image );
}

//
//
//	Device3DVulkan ================================================================================
//
//
void Device3DVulkan::Init()
{
	CreateInstance();

	if ( glfwInit() == GLFW_FALSE )
	{
		throw std::runtime_error( "Error initializing GLFW" );
	}

	if ( glfwVulkanSupported() == GLFW_FALSE )
	{
		throw std::runtime_error( "Error GLFW does not support Vulkan" );
	}

	CreateWindow();

	uint32_t count = 0;
	vkEnumeratePhysicalDevices( m_instance, &count, nullptr );
	if ( count == 0 )
	{
		throw std::runtime_error( "no device found" );
	}

	std::vector<VkPhysicalDevice> devices( count );
	vkEnumeratePhysicalDevices( m_instance, &count, devices.data() );

	m_physicalDevice = devices[0];
	//for ( auto& device : devices )
	//{
	//	if ( IsDeviceSuitable( device, m_surface ) )
	//	{
	//		m_physicalDevice = device;
	//	}
	//}

	if ( m_physicalDevice == VK_NULL_HANDLE )
	{
		throw std::runtime_error( "No suitable device" );
	}

	CreateDeviceAndQueues();
}

void Device3DVulkan::Destroy()
{
	glfwDestroyWindow( m_window );
	m_window = nullptr;
}

void Device3DVulkan::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "VulkanCore";
	appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
	appInfo.pEngineName = "HomeMade";
	appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
#if 0
	if ( enableValidationLayers )
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		createInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}
#endif

	// extensions
	std::vector<const char*> extensions;
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );

	for ( unsigned int i = 0; i < glfwExtensionCount; i++ )
	{
		extensions.push_back( glfwExtensions[i] );
	}

#ifdef _DEBUG
	extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
#endif

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	if ( CheckInstanceExtensions( extensions ) )
	{
		throw std::runtime_error( "missing extension" );
	}

	if ( vkCreateInstance( &createInfo, nullptr, &m_instance ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create instance!" );
	}
}

void Device3DVulkan::DestroyInstance()
{
	vkDestroyInstance( m_instance, nullptr );
}

void Device3DVulkan::CreateWindow()
{
	glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
	glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	m_window = glfwCreateWindow( WIDTH, HEIGHT, "Vulkan", nullptr, nullptr );
	
	if ( glfwCreateWindowSurface( m_instance, m_window, nullptr, &m_surface ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Error while initializing Window Surface." );
	}
}

void Device3DVulkan::DestroyWindow()
{
	vkDestroySurfaceKHR( m_instance, m_surface, nullptr );
	glfwDestroyWindow( m_window );
}

void Device3DVulkan::CreateDeviceAndQueues()
{
	uint32_t queueFamCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &queueFamCount, nullptr );

	VkQueueFamilyProperties * queueProps = (VkQueueFamilyProperties *)alloca( sizeof( VkQueueFamilyProperties ) * queueFamCount );
	vkGetPhysicalDeviceQueueFamilyProperties( m_physicalDevice, &queueFamCount, queueProps );

	int gfxAndPresentQueueIdx = -1;
	int gfxQueueIdx = -1;
	int computeQueueIdx = -1;
	int copyQueueIdx = -1;
	int presentQueueIdx = -1;

	for ( int i = 0; i < (int)queueFamCount; ++i )
	{
		if ( queueProps->queueCount == 0 )
			continue;

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR( m_physicalDevice, i, m_surface, &presentSupport );

		if ( gfxAndPresentQueueIdx < 0 )
		{
			if ( presentSupport && (queueProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT )
			{
				gfxAndPresentQueueIdx = i;
			}
		}

		if ( presentQueueIdx < 0 && presentSupport )
		{
			presentQueueIdx = i;
		}
		if ( gfxQueueIdx < 0 && (queueProps->queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT )
		{
			gfxQueueIdx = i;
		}
		if ( computeQueueIdx < 0 && (queueProps->queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT )
		{
			computeQueueIdx = i;
		}
		if ( copyQueueIdx < 0 && (queueProps->queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT )
		{
			copyQueueIdx = i;
		}
	}

	std::vector<VkDeviceQueueCreateInfo> queue_ci;
	float priority = 1.0f;
	if ( gfxAndPresentQueueIdx >= 0 )
	{
		queue_ci.push_back( VkDeviceQueueCreateInfo{} );
		VkDeviceQueueCreateInfo & ci = queue_ci.back();
		ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = gfxAndPresentQueueIdx;
		ci.queueCount = 1;
		ci.pQueuePriorities = &priority;
	}
	else if ( gfxQueueIdx >= 0 && presentQueueIdx >= 0 )
	{
		queue_ci.push_back( VkDeviceQueueCreateInfo{} );
		VkDeviceQueueCreateInfo & ci = queue_ci.back();
		ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = gfxQueueIdx;
		ci.queueCount = 1;
		ci.pQueuePriorities = &priority;
		
		queue_ci.push_back( VkDeviceQueueCreateInfo{} );
		VkDeviceQueueCreateInfo & ci2 = queue_ci.back();
		ci2.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci2.queueFamilyIndex = presentQueueIdx;
		ci2.queueCount = 1;
		ci2.pQueuePriorities = &priority;
	}

	if ( computeQueueIdx >= 0 )
	{
		queue_ci.push_back( VkDeviceQueueCreateInfo{} );
		VkDeviceQueueCreateInfo & ci = queue_ci.back();
		ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = computeQueueIdx;
		ci.queueCount = 1;
		ci.pQueuePriorities = &priority;
	}

	if ( copyQueueIdx >= 0 )
	{
		queue_ci.push_back( VkDeviceQueueCreateInfo{} );
		VkDeviceQueueCreateInfo & ci = queue_ci.back();
		ci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		ci.queueFamilyIndex = copyQueueIdx;
		ci.queueCount = 1;
		ci.pQueuePriorities = &priority;
	}

	// For now we don't need anything special
	VkPhysicalDeviceFeatures deviceFeatures = {};

	// Create device
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queue_ci.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queue_ci.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;
	deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

#if 0
	if ( enableValidationLayers )
	{
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}
#endif

	if ( vkCreateDevice( m_physicalDevice, &deviceCreateInfo, nullptr, &m_device ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create logical device!" );
	}

	if ( gfxAndPresentQueueIdx >= 0 )
	{
		m_gfxQueue = new DeviceQueueVulkan();
		m_gfxQueue->m_familyIdx = gfxAndPresentQueueIdx;
		vkGetDeviceQueue( m_device, gfxAndPresentQueueIdx, 0, &m_gfxQueue->m_queueNative );
		m_gfxQueue->InitCommandPool( m_device );

		m_presentQueue = m_gfxQueue;
	}
	else
	{
		assert( gfxQueueIdx >= 0 );
		m_gfxQueue = new DeviceQueueVulkan();
		m_gfxQueue->m_familyIdx = gfxQueueIdx;
		vkGetDeviceQueue( m_device, gfxQueueIdx, 0, &m_gfxQueue->m_queueNative );
		m_gfxQueue->InitCommandPool( m_device );

		assert( presentQueueIdx >= 0 );
		m_presentQueue = new DeviceQueueVulkan();
		m_presentQueue->m_familyIdx = presentQueueIdx;
		vkGetDeviceQueue( m_device, presentQueueIdx, 0, &m_presentQueue->m_queueNative );
		m_presentQueue->InitCommandPool( m_device );
	}

	if ( computeQueueIdx >= 0 )
	{
		m_computeQueue = new DeviceQueueVulkan();
		m_computeQueue->m_familyIdx = computeQueueIdx;
		vkGetDeviceQueue( m_device, computeQueueIdx, 0, &m_computeQueue->m_queueNative );
		m_computeQueue->InitCommandPool( m_device );
	}

	if ( copyQueueIdx >= 0 )
	{
		m_copyQueue = new DeviceQueueVulkan();
		m_copyQueue->m_familyIdx = copyQueueIdx;
		vkGetDeviceQueue( m_device, copyQueueIdx, 0, &m_copyQueue->m_queueNative );
		m_copyQueue->InitCommandPool( m_device );
	}
}

void Device3DVulkan::DestroyDeviceAndQueues()
{
	SAFE_DELETE( m_gfxQueue );
	SAFE_DELETE( m_computeQueue );
	SAFE_DELETE( m_copyQueue );
	SAFE_DELETE( m_presentQueue );

	vkDestroyDevice( m_device, nullptr );
}

void Device3DVulkan::CreateSwapChain()
{
	assert( m_surface != VK_NULL_HANDLE );
	
	VkSurfaceCapabilitiesKHR surfaceCaps = {};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_physicalDevice, m_surface, &surfaceCaps );

	// Surface details
	uint32_t surfaceCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &surfaceCount, nullptr );
	if ( surfaceCount > 0 )
	{
		std::vector<VkSurfaceFormatKHR> formats;
		formats.resize( surfaceCount );
		vkGetPhysicalDeviceSurfaceFormatsKHR( m_physicalDevice, m_surface, &surfaceCount, formats.data() );

		m_swapChain.surfaceFormat = formats[0];

		if ( formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED )
		{
			m_swapChain.surfaceFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for ( auto format : formats )
		{
			if ( format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR )
				m_swapChain.surfaceFormat = format;
		}
	}

	// Present mode
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &presentModeCount, nullptr );
	if ( presentModeCount > 0 )
	{
		std::vector<VkPresentModeKHR> presentModes;
		presentModes.resize( presentModeCount );
		vkGetPhysicalDeviceSurfacePresentModesKHR( m_physicalDevice, m_surface, &presentModeCount, presentModes.data() );

		m_swapChain.presentMode = presentModes[0];

		for ( auto& availablePresentMode : presentModes )
		{
			if ( availablePresentMode == preferredPresentMode )
			{
				m_swapChain.presentMode = preferredPresentMode;
				break;
			}
		}
	}

	// Extent
	if ( surfaceCaps.currentExtent.width != std::numeric_limits<uint32_t>::max() )
	{
		m_swapChain.extent = surfaceCaps.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent = { uint32_t( WIDTH ), uint32_t( HEIGHT ) };
		actualExtent.width = std::max( surfaceCaps.minImageExtent.width, std::min( surfaceCaps.maxImageExtent.width, actualExtent.width ) );
		actualExtent.height = std::max( surfaceCaps.minImageExtent.height, std::min( surfaceCaps.maxImageExtent.height, actualExtent.height ) );
		m_swapChain.extent = actualExtent;
	}

	m_swapChain.imageCount = std::min( std::max( surfaceCaps.minImageCount, NUM_BACK_BUFFER ), surfaceCaps.maxImageCount );

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = m_surface;
	createInfo.imageFormat = m_swapChain.surfaceFormat.format;
	createInfo.imageColorSpace = m_swapChain.surfaceFormat.colorSpace;
	createInfo.imageExtent = m_swapChain.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.minImageCount = m_swapChain.imageCount;

	uint32_t queueFamilyIndices[] = { (uint32_t)m_gfxQueue->m_familyIdx, (uint32_t)m_presentQueue->m_familyIdx };

	if ( queueFamilyIndices[0] != queueFamilyIndices[1] )
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = surfaceCaps.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;//VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

	createInfo.presentMode = m_swapChain.presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if ( vkCreateSwapchainKHR( m_device, &createInfo, nullptr, &m_swapChain.m_native ) != VK_SUCCESS )
	{
		throw std::runtime_error( "failed to create swap chain" );
	}

	// Retrieves swap chain images
	uint32_t imageCount;
	vkGetSwapchainImagesKHR( m_device, m_swapChain.m_native, &imageCount, nullptr );
	m_swapChain.m_image = new VkImage[imageCount];
	vkGetSwapchainImagesKHR( m_device, m_swapChain.m_native, &imageCount, m_swapChain.m_image );
}

bool Device3DVulkan::CheckInstanceExtensions( const std::vector<const char*>& requiredExt ) {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> extensions( extensionCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensions.data() );

	std::cout << "available extensions:" << std::endl;

	for ( const auto & extension : extensions )
	{
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	bool allFound = true;
	for ( const auto & req : requiredExt )
	{
		bool found = false;
		for ( const auto & ext : extensions )
		{
			if ( strcmp( ext.extensionName, req ) == 0 )
			{
				found = true;
				break;
			}
		}
		allFound &= found;
	}
	return false;
}

bool Device3DVulkan::CheckDeviceExtensionSupport( VkPhysicalDevice device, const std::vector<const char *> & deviceExtensions ) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );
	std::vector<VkExtensionProperties> availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

	std::set<std::string> requiredExtensions( deviceExtensions.begin(), deviceExtensions.end() );

	for ( auto & ext : availableExtensions )
	{
		requiredExtensions.erase( ext.extensionName );
	}

	return requiredExtensions.empty();
}
