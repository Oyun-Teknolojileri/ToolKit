/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "GpuProgram.h"

#include "Renderer.h"
#include "Shader.h"
#include "TKOpenGL.h"

namespace ToolKit
{

  // GpuProgram
  //////////////////////////////////////////////////////////////////////////

  GpuProgram::GpuProgram() {}

  GpuProgram::GpuProgram(ShaderPtr vertex, ShaderPtr fragment)
  {
    m_shaders.push_back(vertex);
    m_shaders.push_back(fragment);
  }

  GpuProgram::~GpuProgram()
  {
    glDeleteProgram(m_handle);
    m_handle = 0;
  }

  // GpuProgramManager
  //////////////////////////////////////////////////////////////////////////

  void GpuProgramManager::LinkProgram(uint program, uint vertexShaderId, uint fragmentShaderId)
  {
    glAttachShader(program, vertexShaderId);
    glAttachShader(program, fragmentShaderId);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
        char* log = new char[infoLen];
        glGetProgramInfoLog(program, infoLen, nullptr, log);
        GetLogger()->Log(log);
        GetLogger()->WritePlatformConsole(LogType::Memo, log);

        assert(linked);
        SafeDelArray(log);
      }

      glDeleteProgram(program);
    }
  }

  GpuProgramPtr GpuProgramManager::CreateProgram(ShaderPtr vertexShader, ShaderPtr fragmentShader)
  {
    assert(vertexShader);
    assert(fragmentShader);

    vertexShader->Init();
    fragmentShader->Init();

    GpuProgramPtr program = nullptr;

    String tag = GenerateTag((int) vertexShader->GetIdVal(), (int) fragmentShader->GetIdVal());
    if (m_programs.find(tag) == m_programs.end())
    {
      program = MakeNewPtr<GpuProgram>(vertexShader, fragmentShader);
      program->m_tag        = tag;
      program->m_handle     = glCreateProgram();

      LinkProgram(program->m_handle, vertexShader->m_shaderHandle, fragmentShader->m_shaderHandle);
      glUseProgram(program->m_handle);
      for (ubyte slotIndx = 0; slotIndx < Renderer::RHIConstants::TextureSlotCount; slotIndx++)
      {
        GLint loc = glGetUniformLocation(program->m_handle, ("s_texture" + std::to_string(slotIndx)).c_str());
        if (loc != -1)
        {
          glUniform1i(loc, slotIndx);
        }
      }

      m_programs[program->m_tag] = program;
    }
    else
    {
      program = m_programs[tag];
      glUseProgram(program->m_handle);
    }

    // Register uniform locations
    for (ShaderPtr shader : m_programs[tag]->m_shaders)
    {
      for (Uniform uniform : shader->m_uniforms)
      {
        GLint loc = glGetUniformLocation(program->m_handle, GetUniformName(uniform));
        //TODO
        if (loc == -1)
        {
          volatile int y = 5;
        }
        program->m_uniformLocations[uniform] = loc;
      }

      // Array uniforms
      for (Shader::ArrayUniform arrayUniform : shader->m_arrayUniforms)
      {
        program->m_arrayUniformLocations[arrayUniform.uniform].reserve(arrayUniform.size);
        for (int i = 0; i < arrayUniform.size; ++i)
        {
          String uniformName = GetUniformName(arrayUniform.uniform);
          uniformName        = uniformName + "[" + std::to_string(i) + "]";
          GLint loc          = glGetUniformLocation(program->m_handle, uniformName.c_str());
          // TODO
          if (loc == -1)
          {
            volatile int y = 5;
          }
          program->m_arrayUniformLocations[arrayUniform.uniform].push_back(loc);
        }
      }
    }

    return program;
  }

  void GpuProgramManager::FlushPrograms() { m_programs.clear(); }

  String GpuProgramManager::GenerateTag(int vertexShaderId, int fragmentShaderId)
  {
    return std::to_string(vertexShaderId) + "|" + std::to_string(fragmentShaderId);
  }

} // namespace ToolKit