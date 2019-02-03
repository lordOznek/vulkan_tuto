#include <stdafx.h>
#include "core/vulkan/Device3D_vulkan.h"

int main() {
	int exitCode = EXIT_SUCCESS;

	Device3DVulkan * device = new Device3DVulkan;
	device->Init();

	try
	{
		
	}
	catch ( const std::runtime_error& e )
	{
		std::cerr << e.what() << std::endl;
		exitCode = EXIT_FAILURE;
		goto terminate;
	}

terminate:
	device->Destroy();
	delete device;

	return exitCode;
}