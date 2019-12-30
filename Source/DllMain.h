#pragma once

#include <functional>

#define ABT_EXPORT __declspec(dllexport)

extern "C"
{
	struct MainParams
	{
		const char* programName;
		int windowWidth;
		int windowHeight;
		int fps;
		bool hidden;
		void (*FrameCallback)(int fps);
	};

	enum class DllErrorCodes
	{
		Failed,
		Succeeded
	};

	// Initiate SDL2 return success or error.
	ABT_EXPORT DllErrorCodes AbtMain(MainParams args);
}