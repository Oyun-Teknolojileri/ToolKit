#include "stdafx.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Drawable.h"
#include "Texture.h"
#include "Directional.h"
#include "Node.h"
#include "Material.h"
#include "Surface.h"
#include "Skeleton.h"
#include "DebugNew.h"

namespace ToolKit
{

#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

	Renderer::Renderer()
	{
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Render(Drawable* object, Camera* cam, const LightRawPtrArray& lights)
	{
		if (object->m_mesh->IsSkinned())
		{
			RenderSkinned(object, cam);
			return;
		}

		object->m_mesh->Init();

		MeshRawPtrArray meshes;
		object->m_mesh->GetAllMeshes(meshes);

		m_cam = cam;
		m_lights = lights;
		SetProjectViewModel(object, cam);

		for (Mesh* mesh : meshes)
		{
			m_mat = mesh->m_material.get();

			ProgramPtr prg = CreateProgram(m_mat->m_vertexShader, m_mat->m_fragmetShader);
			BindProgram(prg);
			FeedUniforms(prg);

			RenderState rs = *m_mat->GetRenderState();
			SetRenderState(rs);

			glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
			SetVertexLayout(VertexLayout::Mesh);

			if (mesh->m_indexCount != 0)
			{
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
				glDrawElements((GLenum)rs.drawType, mesh->m_indexCount, GL_UNSIGNED_INT, nullptr);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else
			{
				glDrawArrays((GLenum)rs.drawType, 0, mesh->m_vertexCount);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}

			SetVertexLayout(VertexLayout::None);
		}
	}

	void Renderer::RenderSkinned(Drawable* object, Camera* cam)
	{
		object->m_mesh->Init();
		SetProjectViewModel(object, cam);

		ShaderPtr skinShader = GetShaderManager()->Create(ShaderPath("defaultSkin.shader"));
		ShaderPtr fragShader = GetShaderManager()->Create(ShaderPath("defaultFragment.shader"));
		ProgramPtr skinProg = CreateProgram(skinShader, fragShader);
		BindProgram(skinProg);
		FeedUniforms(skinProg);

		Skeleton* skeleton = static_cast<SkinMesh*> (object->m_mesh.get())->m_skeleton;
		for (int i = 0; i < (int)skeleton->m_bones.size(); i++)
		{
			Bone* bone = skeleton->m_bones[i];
			String shaderName = "bones[" + std::to_string(i) + "].transform";
			GLint loc = glGetUniformLocation(skinProg->m_handle, shaderName.c_str());
			Mat4 transform = bone->m_node->GetTransform(TransformationSpace::TS_WORLD);
			glUniformMatrix4fv(loc, 1, false, &transform[0][0]);

			shaderName = "bones[" + std::to_string(i) + "].bindPose";
			loc = glGetUniformLocation(skinProg->m_handle, shaderName.c_str());
			glUniformMatrix4fv(loc, 1, false, &bone->m_inverseWorldMatrix[0][0]);
		}

		MeshPtrArray meshes;
		meshes.push_back(object->m_mesh);
		for (int i = 0; i < (int)object->m_mesh->m_subMeshes.size(); i++)
		{
			meshes.push_back(object->m_mesh->m_subMeshes[i]);
		}

		for (MeshPtr mesh : meshes)
		{
			RenderState rs = *mesh->m_material->GetRenderState();
			SetRenderState(rs);

			glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
			SetVertexLayout(VertexLayout::SkinMesh);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
			glDrawElements((GLenum)rs.drawType, mesh->m_indexCount, GL_UNSIGNED_INT, nullptr);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			SetVertexLayout(VertexLayout::None);
		}
	}

	void Renderer::Render2d(Surface* object, glm::ivec2 screenDimensions)
	{
		object->Init();

		std::shared_ptr<Shader> vertexShader = GetShaderManager()->Create(ShaderPath("defaultVertex.shader"));
		std::shared_ptr<Shader> fragShader = GetShaderManager()->Create(ShaderPath("defaultFragment.shader"));
		std::shared_ptr<Program> prog = CreateProgram(vertexShader, fragShader);
		BindProgram(prog);

		RenderState rs = *object->m_mesh->m_material->GetRenderState();
		SetRenderState(rs);

		GLint pvloc = glGetUniformLocation(prog->m_handle, "ProjectViewModel");
		Mat4 pm = glm::ortho(0.0f, (float)screenDimensions.x, 0.0f, (float)screenDimensions.y, 0.0f, 100.0f);
		Mat4 mul = pm * object->m_node->GetTransform(TransformationSpace::TS_WORLD);
		glUniformMatrix4fv(pvloc, 1, false, &mul[0][0]);

		glBindBuffer(GL_ARRAY_BUFFER, object->m_mesh->m_vboVertexId);
		SetVertexLayout(VertexLayout::Mesh);

		glDrawArrays((GLenum)rs.drawType, 0, object->m_mesh->m_vertexCount);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		SetVertexLayout(VertexLayout::None);
	}

	void Renderer::Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions)
	{
		Surface* surface = object->GetCurrentSurface();

		Node* backup = surface->m_node;
		surface->m_node = object->m_node;

		Render2d(surface, screenDimensions);

		surface->m_node = backup;
	}

	void Renderer::SetRenderState(RenderState state)
	{
		if (m_renderState.cullMode != state.cullMode)
		{
			if (state.cullMode == CullingType::TwoSided)
			{
				glDisable(GL_CULL_FACE);
			}

			if (state.cullMode == CullingType::Front)
			{
				if (m_renderState.cullMode == CullingType::TwoSided)
				{
					glEnable(GL_CULL_FACE);
				}
				glCullFace(GL_FRONT);
			}

			if (state.cullMode == CullingType::Back)
			{
				if (m_renderState.cullMode == CullingType::TwoSided)
				{
					glEnable(GL_CULL_FACE);
				}
				glCullFace(GL_BACK);
			}

			m_renderState.cullMode = state.cullMode;
		}

		if (m_renderState.depthTestEnabled != state.depthTestEnabled)
		{
			if (state.depthTestEnabled)
				glEnable(GL_DEPTH_TEST);
			else
				glDisable(GL_DEPTH_TEST);
			m_renderState.depthTestEnabled = state.depthTestEnabled;
		}

		if (m_renderState.blendFunction != state.blendFunction)
		{
			switch (state.blendFunction)
			{
			case BlendFunction::NONE:
				glDisable(GL_BLEND);
				break;
			case BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;
			}

			m_renderState.blendFunction = state.blendFunction;
		}

		if (m_renderState.diffuseTexture != state.diffuseTexture && state.diffuseTextureInUse)
		{
			m_renderState.diffuseTexture = state.diffuseTexture;
			glBindTexture(GL_TEXTURE_2D, m_renderState.diffuseTexture);
		}

		if (m_renderState.cubeMap != state.cubeMap && state.cubeMapInUse)
		{
			m_renderState.cubeMap = state.cubeMap;
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_renderState.cubeMap);
		}

		if (m_renderState.lineWidth != state.lineWidth)
		{
			m_renderState.lineWidth = state.lineWidth;
			glLineWidth(m_renderState.lineWidth);
		}
	}

	void Renderer::SetRenderTarget(RenderTarget* renderTarget, bool clear)
	{
		if (m_renderTarget == renderTarget)
		{
			return;
		}

		if (renderTarget != nullptr)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->m_frameBufferId);
			glViewport(0, 0, renderTarget->m_width, renderTarget->m_height);

			if (clear)
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			}
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, m_windowWidth, m_windowHeight);
		}

		m_renderTarget = renderTarget;
	}

	void Renderer::DrawFullQuad(ShaderPtr fragmentShader)
	{
		static ShaderPtr fullQuadVert = GetShaderManager()->Create(ShaderPath("fullQuadVert.shader"));
		static MaterialPtr material = std::make_shared<Material>();
		material->UnInit();

		material->m_vertexShader = fullQuadVert;
		material->m_fragmetShader = fragmentShader;
		material->Init();
		
		static Quad quad;
		quad.m_mesh->m_material = material;

		static Camera dummy;

		Render(&quad, &dummy);
	}

	void Renderer::SetProjectViewModel(Drawable* object, Camera* cam)
	{
		m_view = cam->GetViewMatrix();
		m_project = cam->GetData().projection;
		m_model = object->m_node->GetTransform(TransformationSpace::TS_WORLD);
	}

	void Renderer::BindProgram(ProgramPtr program)
	{
		if (m_currentProgram == program->m_handle)
			return;
		m_currentProgram = program->m_handle;
		glUseProgram(program->m_handle);
	}

	void Renderer::LinkProgram(GLuint program, GLuint vertexP, GLuint fragmentP)
	{
		glAttachShader(program, vertexP);
		glAttachShader(program, fragmentP);

		glLinkProgram(program);

		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			assert(linked);
			GLint infoLen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen > 1)
			{
				char* log = new char[infoLen];
				glGetProgramInfoLog(program, infoLen, nullptr, log);
				Logger::GetInstance()->Log(log);

				SafeDelArray(log);
			}

			glDeleteProgram(program);
		}
	}

	ProgramPtr Renderer::CreateProgram(ShaderPtr vertex, ShaderPtr fragment)
	{
		assert(vertex);
		assert(fragment);

		ProgramPtr program = std::make_shared<Program>(vertex, fragment);
		if (m_programs.find(program->m_tag) == m_programs.end())
		{
			program->m_handle = glCreateProgram();
			LinkProgram(program->m_handle, vertex->m_shaderHandle, fragment->m_shaderHandle);
			m_programs[program->m_tag] = program;
		}

		return m_programs[program->m_tag];
	}

	void Renderer::FeedUniforms(ProgramPtr program)
	{
		for (ShaderPtr shader : program->m_shaders)
		{

			// Built-in variables.
			for (Uniform uni : shader->m_uniforms)
			{
				switch (uni)
				{
				case Uniform::PROJECT_MODEL_VIEW:
				{
					GLint loc = glGetUniformLocation(program->m_handle, "ProjectViewModel");
					Mat4 mul = m_project * m_view * m_model;
					glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
				}
				break;
				case Uniform::MODEL:
				{
					GLint loc = glGetUniformLocation(program->m_handle, "Model");
					glUniformMatrix4fv(loc, 1, false, &m_model[0][0]);
				}
				break;
				case Uniform::INV_TR_MODEL:
				{
					GLint loc = glGetUniformLocation(program->m_handle, "InverseTransModel");
					Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
					glUniformMatrix4fv(loc, 1, false, &invTrModel[0][0]);
				}
				break;
				case Uniform::LIGHT_DATA:
				{
					if (m_lights.empty())
						break;

					for (size_t i = 0; i < m_lights.size(); i++)
					{
						Light::LightData data = m_lights[i]->GetData();

						String shaderName = "LightData.pos[" + std::to_string(i) + "]";
						GLint loc = glGetUniformLocation(program->m_handle, shaderName.c_str());
						glUniform3fv(loc, 1, &data.pos.x);
						shaderName = "LightData.dir[" + std::to_string(i) + "]";
						loc = glGetUniformLocation(program->m_handle, shaderName.c_str());
						glUniform3fv(loc, 1, &data.dir.x);
						shaderName = "LightData.color[" + std::to_string(i) + "]";
						loc = glGetUniformLocation(program->m_handle, shaderName.c_str());
						glUniform3fv(loc, 1, &data.color.x);
						shaderName = "LightData.intensity[" + std::to_string(i) + "]";
						loc = glGetUniformLocation(program->m_handle, shaderName.c_str());
						glUniform1f(loc, data.intensity);
					}

					GLint loc = glGetUniformLocation(program->m_handle,"LightData.activeCount");
					glUniform1i(loc, (int)m_lights.size());
				}
				break;
				case Uniform::CAM_DATA:
				{
					if (m_cam == nullptr)
						break;

					Camera::CamData data = m_cam->GetData();
					GLint loc = glGetUniformLocation(program->m_handle, "CamData.pos");
					glUniform3fv(loc, 1, &data.pos.x);
					loc = glGetUniformLocation(program->m_handle, "CamData.dir");
					glUniform3fv(loc, 1, &data.dir.x);
				}
				break;
				case Uniform::COLOR:
				{
					if (m_mat == nullptr)
						return;

					Vec3 color = m_mat->m_color;
					GLint loc = glGetUniformLocation(program->m_handle, "Color");
					glUniform3fv(loc, 1, &color.x);
				}
				break;
				case Uniform::FRAME_COUNT:
				{
					GLint loc = glGetUniformLocation(program->m_handle, "FrameCount");
					glUniform1ui(loc, m_frameCount);
				}
				break;
				default:
					assert(false);
					break;
				}
			}

			// Custom variables.
			for (auto var : shader->m_shaderParams)
			{
				GLint loc = glGetUniformLocation(program->m_handle, var.first.c_str());
				if (loc == -1)
				{
					continue;
				}

				switch (var.second.GetType())
				{
				case ShaderVariant::VariantType::Float:
					glUniform1f(loc, var.second.GetVar<float>());
					break;
				case ShaderVariant::VariantType::Int:
					glUniform1i(loc, var.second.GetVar<int>());
					break;
				case ShaderVariant::VariantType::Vec3:
					glUniform3fv(loc, 1, (float*)&var.second.GetVar<Vec3>());
					break;
				case ShaderVariant::VariantType::Vec4:
					glUniform4fv(loc, 1, (float*)&var.second.GetVar<Vec4>());
					break;
				case ShaderVariant::VariantType::Mat3:
					glUniformMatrix3fv(loc, 1, false, (float*)&var.second.GetVar<Mat3>());
					break;
				case ShaderVariant::VariantType::Mat4:
					glUniformMatrix4fv(loc, 1, false, (float*)&var.second.GetVar<Mat4>());
					break;
				default:
					assert(false && "Invalid type.");
					break;
				}
			}

		}
	}

	void Renderer::SetVertexLayout(VertexLayout layout)
	{
		if (layout == VertexLayout::None)
		{
			for (int i = 0; i < 6; i++)
			{
				glDisableVertexAttribArray(i);
			}
		}

		if (layout == VertexLayout::Mesh)
		{
			GLuint offset = 0;
			glEnableVertexAttribArray(0); // Vertex
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
			offset += 3 * sizeof(float);

			glEnableVertexAttribArray(1); // Normal
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
			offset += 3 * sizeof(float);

			glEnableVertexAttribArray(2); // Texture
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
			offset += 2 * sizeof(float);

			glEnableVertexAttribArray(3); // BiTangent
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
		}

		if (layout == VertexLayout::SkinMesh)
		{
			GLuint offset = 0;
			glEnableVertexAttribArray(0); // Vertex
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), 0);
			offset += 3 * sizeof(float);

			glEnableVertexAttribArray(1); // Normal
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
			offset += 3 * sizeof(float);

			glEnableVertexAttribArray(2); // Texture
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
			offset += 2 * sizeof(float);

			glEnableVertexAttribArray(3); // BiTangent
			glVertexAttribIPointer(3, 3, GL_UNSIGNED_INT, sizeof(SkinVertex), BUFFER_OFFSET(offset));
			offset += 3 * sizeof(uint);

			glEnableVertexAttribArray(4); // Bones
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
			offset += 4 * sizeof(float);

			glEnableVertexAttribArray(5); // Weights
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
		}
	}

}
