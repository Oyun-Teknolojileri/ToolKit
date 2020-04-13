#include "stdafx.h"
#include "Shader.h"
#include "ToolKit.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"
#include <vector>
#include "DebugNew.h"

namespace ToolKit
{

	Shader::Shader()
	{
	}

	Shader::Shader(String file)
	{
		m_file = file;
	}

	Shader::~Shader()
	{
		UnInit();
	}

	void Shader::Load()
	{
		if (m_loaded)
			return;

		rapidxml::file<> file(m_file.c_str());
		rapidxml::xml_document<> doc;
		doc.parse<rapidxml::parse_full>(file.data());

		rapidxml::xml_node<>* rootNode = doc.first_node("shader");
		if (rootNode == nullptr)
			return;

		for (rapidxml::xml_node<>* node = rootNode->first_node(); node; node = node->next_sibling())
		{
			if (String("type").compare(node->name()) == 0)
			{
				rapidxml::xml_attribute<>* attr = node->first_attribute("name");
				if (String("vertexShader").compare(attr->value()) == 0)
				{
					m_type = GL_VERTEX_SHADER;
				}
				else if (String("fragmentShader").compare(attr->value()) == 0)
				{
					m_type = GL_FRAGMENT_SHADER;
				}
				else
				{
					assert(false);
				}
			}

			if (String("uniform").compare(node->name()) == 0)
			{
				rapidxml::xml_attribute<>* attr = node->first_attribute();
				if (String("ProjectViewModel").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
				}
				else if (String("Model").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::MODEL);
				}
				else if (String("InverseTransModel").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::INV_TR_MODEL);
				}
				else if (String("LightData").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::LIGHT_DATA);
				}
				else if (String("CamData").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::CAM_DATA);
				}
				else if (String("Color").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::COLOR);
				}
				else if (String("FrameCount").compare(attr->value()) == 0)
				{
					m_uniforms.push_back(Uniform::FRAME_COUNT);
				}
				else
				{
					assert(false);
				}
			}

			if (String("source").compare(node->name()) == 0)
			{
				m_source = node->first_node()->value();
			}
		}

		m_loaded = true;
	}

	void Shader::Init(bool flushClientSideArray)
	{
		if (m_initiated)
			return;

		m_shaderHandle = glCreateShader(m_type);
		if (m_shaderHandle == 0)
			return;

		const char* str = m_source.c_str();
		glShaderSource(m_shaderHandle, 1, &str, nullptr);
		glCompileShader(m_shaderHandle);

		GLint compiled;
		glGetShaderiv(m_shaderHandle, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			assert(compiled);
			GLint infoLen = 0;
			glGetShaderiv(m_shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen > 1)
			{
				char* log = new char[infoLen];
				glGetShaderInfoLog(m_shaderHandle, infoLen, nullptr, log);
				Logger::GetInstance()->Log(log);

				SafeDelArray(log);
			}

			glDeleteShader(m_shaderHandle);
			return;
		}

		if (flushClientSideArray)
			m_source.clear();

		m_initiated = true;
	}

	void Shader::UnInit()
	{
		glDeleteShader(m_shaderHandle);
		m_initiated = false;
	}

	void ShaderManager::Init()
	{
		ResourceManager::Init();

		char vShaderStr[] =
			"#version 300 es\n"
			"in vec3 vPosition;"
			"in vec3 vNormal;"
			"in vec2 vTexture;"
			"in vec3 vBiTan;"
			"uniform mat4 ProjectViewModel;"
			"out vec3 v_normal;"
			"out vec2 v_texCoord;"
			"void main()"
			"{"
			"   gl_Position = ProjectViewModel * vec4(vPosition, 1.0);"
			"   v_texCoord = vTexture;"
			"   v_normal = vNormal;"
			"}";

		char skinShaderStr[] =
			"#version 300 es\n"
			"struct Bone"
			"{"
			"   mat4 transform;"
			"   mat4 bindPose;"
			"};"
			"in vec3 vPosition;"
			"in vec3 vNormal;"
			"in vec2 vTexture;"
			"in vec3 vBiTan;"
			"in uvec4 vBones;"
			"in vec4 vWeights;"
			"uniform mat4 ProjectViewModel;"
			"uniform Bone bones[64];"
			"out vec3 v_normal;"
			"out vec2 v_texCoord;"
			"out vec3 v_bitan;"
			"void main()"
			"{"
			"   gl_Position = bones[vBones.x].transform * bones[vBones.x].bindPose * vec4(vPosition, 1.0) * vWeights.x;"
			"   gl_Position += bones[vBones.y].transform * bones[vBones.y].bindPose * vec4(vPosition, 1.0) * vWeights.y;"
			"   gl_Position += bones[vBones.z].transform * bones[vBones.z].bindPose * vec4(vPosition, 1.0) * vWeights.z;"
			"   gl_Position += bones[vBones.w].transform * bones[vBones.w].bindPose * vec4(vPosition, 1.0) * vWeights.w;"
			"   gl_Position = ProjectViewModel * gl_Position;"
			"   v_texCoord = vTexture;"
			"   v_normal = vNormal;"
			"   v_bitan = vBiTan;"
			"}";

		char fShaderStr[] =
			"#version 300 es\n"
			"precision mediump float;"
			"in vec3 v_normal;"
			"in vec2 v_texCoord;"
			"out vec4 v_fragColor;"
			"uniform sampler2D s_texture;"
			"void main()"
			"{"
			"  vec2 texFixedc = vec2(v_texCoord.x, 1.0 - v_texCoord.y);"
			"  v_fragColor = texture(s_texture, v_texCoord);"
			"}";

		std::shared_ptr<Shader> vertex = std::make_shared<Shader>();
		vertex->m_source = vShaderStr;
		vertex->m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
		vertex->Init();

		m_storage[ShaderPath("defaultVertex.shader")] = vertex;

		std::shared_ptr<Shader> fragment = std::make_shared<Shader>();
		fragment->m_source = fShaderStr;
		fragment->m_type = GL_FRAGMENT_SHADER;
		fragment->Init();

		m_storage[ShaderPath("defaultFragment.shader")] = fragment;

		std::shared_ptr<Shader> skin = std::make_shared<Shader>();
		skin->m_source = skinShaderStr;
		skin->m_uniforms.push_back(Uniform::PROJECT_MODEL_VIEW);
		skin->Init();

		m_storage[ShaderPath("defaultSkin.shader")] = skin;
	}

}
