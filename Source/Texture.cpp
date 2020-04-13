#include "stdafx.h"
#include "Texture.h"
#include "lodepng.h"
#include "LoadPng/lodepng.h"
#include "LoadPng/lodepng.cpp"
#include "DebugNew.h"

namespace ToolKit
{

	Texture::Texture()
	{
	}

	Texture::Texture(String file)
	{
		m_file = file;
		m_textureId = 0;
	}

	Texture::~Texture()
	{
		UnInit();
	}

	void Texture::Load()
	{
		if (m_loaded)
			return;

		if (!lodepng::decode(m_image, m_width, m_height, m_file))
			m_loaded = true;
	}

	void Texture::Init(bool flushClientSideArray)
	{
		if (m_initiated)
			return;

		if (m_image.size() <= 0 || m_width <= 0 || m_height <= 0)
			return;

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (GLEW_EXT_texture_filter_anisotropic)
		{
			float aniso = 0.0f;
			glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
		}

		if (flushClientSideArray)
			m_image.clear();
		m_initiated = true;
	}

	void Texture::UnInit()
	{
		glDeleteTextures(1, &m_textureId);
		m_initiated = false;
	}

	CubeMap::CubeMap()
	{
	}

	CubeMap::CubeMap(String file)
		: Texture(file)
	{
	}

	CubeMap::~CubeMap()
	{
		UnInit();
	}

	void CubeMap::Load()
	{
		if (m_loaded)
			return;

		m_images.resize(6);
		size_t pos = m_file.find("px.png");
		if (pos == String::npos)
		{
			Logger::GetInstance()->Log("Inapropriate postfix. Looking for \"px.png\": " + m_file);
			return;
		}

		String file = m_file.substr(0, pos);

		for (int i = 0; i < 6; i++)
		{
			String postfix = "px.png";
			switch (i)
			{
			case 1:
				postfix = "nx.png";
				break;
			case 2:
				postfix = "py.png";
				break;
			case 3:
				postfix = "ny.png";
				break;
			case 4:
				postfix = "pz.png";
				break;
			case 5:
				postfix = "nz.png";
				break;
			}

			if (lodepng::decode(m_images[i], m_width, m_height, file + postfix))
			{
				Logger::GetInstance()->Log("Missing file: " + file + postfix);
				Logger::GetInstance()->Log("Cube map loading requires additional 5 png files with postfix \"nx py ny pz nz\".");
				m_loaded = false;

				m_images.clear();
				return;
			}
		}

		m_loaded = true;
	}

	void CubeMap::Init(bool flushClientSideArray)
	{
		if (m_initiated)
			return;

		if (!m_loaded)
			return;

		// Sanity check.
		if (m_images[0].size() <= 0 || m_width <= 0 || m_height <= 0)
			return;

		glGenTextures(1, &m_textureId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);

		uint sides[6] =
		{
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
		};

		for (int i = 0; i < 6; i++)
		{
			glTexImage2D(sides[i], 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_images[i].data());
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		if (flushClientSideArray)
			m_image.clear();
		m_initiated = true;
	}

	void CubeMap::UnInit()
	{
		Texture::UnInit();

		m_initiated = false;
	}

	RenderTarget::RenderTarget(uint width, uint height)
	{
		m_width = width;
		m_height = height;
		m_frameBufferId = 0;
		m_depthBufferId = 0;
	}

	RenderTarget::~RenderTarget()
	{
		UnInit();
	}

	void RenderTarget::Load()
	{
	}

	void RenderTarget::Init(bool flushClientSideArray)
	{
		if (m_initiated)
			return;

		if (m_width <= 0 || m_height <= 0)
			return;

		// Create frame buffer color texture
		glGenTextures(1, &m_textureId);

		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glGenFramebuffers(1, &m_frameBufferId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

		// Attach 2D texture to this FBO
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);

		glGenRenderbuffers(1, &m_depthBufferId);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);

		// Attach depth buffer to FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferId);

		GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, DrawBuffers);

		GLenum stat = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
		if (stat != GL_FRAMEBUFFER_COMPLETE)
		{
			m_initiated = false;
			glDeleteTextures(1, &m_textureId);
			glDeleteFramebuffers(1, &m_frameBufferId);
			glDeleteRenderbuffers(1, &m_depthBufferId);
		}
		else
		{
			m_initiated = true;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderTarget::UnInit()
	{
		Texture::UnInit();

		glDeleteFramebuffers(1, &m_frameBufferId);
		glDeleteRenderbuffers(1, &m_depthBufferId);
		m_initiated = false;
	}

}
