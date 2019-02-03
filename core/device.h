#pragma once

class IDevice3D
{
public:
	enum QueueType
	{
		GraphicsQueue = 0,
		ComputeQueue,
		CopyQueue,
		PresentQueue,

		QueueCount,
	};

public:
	virtual void Init() = 0;
	virtual void Destroy() = 0;

};