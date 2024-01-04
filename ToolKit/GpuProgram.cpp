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

  int GpuProgram::GetUniformLocation(Uniform uniform, int index) const
  {
    if (index == -1)
    {
      const auto& itr = m_uniformLocations.find(uniform);
      if (itr != m_uniformLocations.end())
      {
        return itr->second;
      }
      else
      {
        return -1;
      }
    }
    else
    {
      // Uniform is an array
      const auto& itr = m_arrayUniformLocations.find(uniform);
      if (itr != m_arrayUniformLocations.end())
      {
        const IntArray& locs = itr->second;
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
    const auto& itr = m_shaderParamsUniformLocations.find(uniformName);
    if (itr != m_shaderParamsUniformLocations.end())
    {
      return itr->second;
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

  bool GpuProgram::UpdateUniform(const ShaderUniform& uniform)
  {
    // TODO: Cihan - cost is high, only update if dirty don't check equality
    return true;

    const auto& itr = m_customUniforms.find(uniform.m_name);
    if (itr != m_customUniforms.end())
    {
      if (itr->second.m_value == uniform.m_value)
      {
        return false;
      }
    }

    m_customUniforms[uniform.m_name] = uniform;
    return true;
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