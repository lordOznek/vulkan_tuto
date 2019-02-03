#include <stdafx.h>
#include "DeviceQueue_vulkan.h"

void DeviceQueueVulkan::InitCommandPool( VkDevice & device)
{
	VkCommandPoolCreateInfo cpInfo = {};
	cpInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpInfo.queueFamilyIndex = m_familyIdx;

	if ( vkCreateCommandPool( device, &cpInfo, nullptr, &m_commandPool ) != VK_SUCCESS )
	{
		throw std::runtime_error( "Cannot create command pool" );
	}
}
