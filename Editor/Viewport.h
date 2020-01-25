#pragma once

#include "Directional.h"

namespace ToolKit
{
	class Viewport
	{
	public:
		Viewport(unsigned int width, unsigned int height, Camera* cam)
			: m_width(width), m_height(height), m_camera(cam)
		{
		}

		int m_width = 640;
		int m_height = 480;
		Camera* m_camera = nullptr;
	};
}