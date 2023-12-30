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

  int GpuProgram::GetUniformLocation(Uniform uniform, int index)
  {
    if (index == -1)
    {
      if (m_uniformLocations.find(uniform) != m_uniformLocations.end())
      {
        return m_uniformLocations[uniform];
      }
      else
      {
        return -1;
      }
    }
    else
    {
      // Uniform is an array
      if (m_arrayUniformLocations.find(uniform) != m_arrayUniformLocations.end())
      {
        IntArray& locs = m_arrayUniformLocations[uniform];
        if (locs.size() > index)
        {
          return locs[index];
        }
      }

      return -1;
    }
  }

  int GpuProgram::GetShaderParamUniformLoc(const String& uniformName)
  {
    if (m_shaderParamsUniformLocations.find(uniformName) != m_shaderParamsUniformLocations.end())
    {
      return m_shaderParamsUniformLocations[uniformName];
    }
    else
    {
      // Note: Assuming the shader program is in use
      GLint loc                                   = glGetUniformLocation(m_handle, uniformName.c_str());
      m_shaderParamsUniformLocations[uniformName] = loc;
      return loc;
    }

    return -1;
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

    GpuProgramPtr program;
    auto progIter = m_programs.find({vertexShader->GetIdVal(), fragmentShader->GetIdVal()});

    if (progIter == m_programs.end())
    {
      program           = MakeNewPtr<GpuProgram>(vertexShader, fragmentShader);
      program->m_handle = glCreateProgram();

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

      // Register uniform locations
      for (ShaderPtr shader : program->m_shaders)
      {
        for (Uniform uniform : shader->m_uniforms)
        {
          GLint loc                            = glGetUniformLocation(program->m_handle, GetUniformName(uniform));
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
            program->m_arrayUniformLocations[arrayUniform.uniform].push_back(loc);
          }
        }
      }

      m_programs[{vertexShader->GetIdVal(), fragmentShader->GetIdVal()}] = program;
    }
    else
    {
      program = progIter->second;
    }

    return program;
  }

  void GpuProgramManager::FlushPrograms() { m_programs.clear(); }

} // namespace ToolKit