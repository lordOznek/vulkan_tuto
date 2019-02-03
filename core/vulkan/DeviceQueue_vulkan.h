#pragma once
#include <vulkan/vulkan.h>

class DeviceQueueVulkan
{
public:
	void InitCommandPool( VkDevice & device );

public:
	int m_familyIdx;
	VkQueue m_queueNative;
	VkCommandPool m_commandPool;
};