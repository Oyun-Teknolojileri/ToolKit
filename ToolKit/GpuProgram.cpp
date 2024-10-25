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
  //////////////////////////////////////////

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

  int GpuProgram::GetDefaultUniformLocation(Uniform uniform, int index)
  {
    if (index == -1)
    {
      const auto& itr = m_defaultUniformLocation.find(uniform);
      if (itr != m_defaultUniformLocation.end())
      {
        return itr->second;
      }
    }
    else
    {
      // Uniform is an array
      const auto& itr = m_defaultArrayUniformLocations.find(uniform);
      if (itr != m_defaultArrayUniformLocations.end())
      {
        const IntArray& locs = itr->second;
        if (locs.size() > index)
        {
          return locs[index];
        }
      }
    }

    return -1;
  }

  int GpuProgram::GetCustomUniformLocation(ShaderUniform& shaderUniform)
  {
    if (!shaderUniform.m_thisUniformIsSearchedInGPUProgram)
    {
      shaderUniform.m_thisUniformIsSearchedInGPUProgram = true;

      GLint loc                                         = glGetUniformLocation(m_handle, shaderUniform.m_name.c_str());
      if (loc == -1)
      {
        TK_WRN("Uniform: \"%s\" does not exist in program!", shaderUniform.m_name.c_str());
      }

      shaderUniform.m_locInGPUProgram = loc;
    }

    return shaderUniform.m_locInGPUProgram;
  }

  void GpuProgram::UpdateCustomUniform(const String& uniformName, const UniformValue& val)
  {
    auto paramItr = m_customUniforms.find(uniformName);
    if (paramItr == m_customUniforms.end())
    {
      m_customUniforms[uniformName] = ShaderUniform(uniformName, val);
    }
    else
    {
      paramItr->second = val;
    }
  }

  void GpuProgram::UpdateCustomUniform(const ShaderUniform& uniform)
  {
    auto paramItr = m_customUniforms.find(uniform.m_name);
    if (paramItr == m_customUniforms.end())
    {
      m_customUniforms[uniform.m_name] = uniform;
    }
    else
    {
      paramItr->second = uniform.m_value;
    }
  }

  // GpuProgramManager
  //////////////////////////////////////////

  GpuProgramManager::~GpuProgramManager() { FlushPrograms(); }

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

  const GpuProgramPtr& GpuProgramManager::CreateProgram(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader)
  {
    assert(vertexShader);
    assert(fragmentShader);

    vertexShader->Init();
    fragmentShader->Init();

    const auto& progIter = m_programs.find({vertexShader->m_shaderHandle, fragmentShader->m_shaderHandle});
    if (progIter == m_programs.end())
    {
      GpuProgramPtr program = MakeNewPtr<GpuProgram>(vertexShader, fragmentShader);
      program->m_handle     = glCreateProgram();

      LinkProgram(program->m_handle, vertexShader->m_shaderHandle, fragmentShader->m_shaderHandle);
      glUseProgram(program->m_handle);
      for (ubyte slotIndx = 0; slotIndx < RHIConstants::TextureSlotCount; slotIndx++)
      {
        GLint loc = glGetUniformLocation(program->m_handle, ("s_texture" + std::to_string(slotIndx)).c_str());
        if (loc != -1)
        {
          glUniform1i(loc, slotIndx);
        }
      }

      GLuint loc = glGetUniformBlockIndex(program->m_handle, "LightDataBuffer");
      if (loc != GL_INVALID_INDEX)
      {
        glUniformBlockBinding(program->m_handle, loc, 0);
      }
      loc = glGetUniformBlockIndex(program->m_handle, "ActiveLightIndicesBuffer");
      if (loc != GL_INVALID_INDEX)
      {
        glUniformBlockBinding(program->m_handle, loc, 1);
      }

      // Register default uniform locations
      for (ShaderPtr shader : program->m_shaders)
      {
        for (const Uniform& uniform : shader->m_uniforms)
        {
          GLint loc                                  = glGetUniformLocation(program->m_handle, GetUniformName(uniform));
          program->m_defaultUniformLocation[uniform] = loc;
        }

        // Array uniforms
        for (Shader::ArrayUniform arrayUniform : shader->m_arrayUniforms)
        {
          program->m_defaultArrayUniformLocations[arrayUniform.uniform].reserve(arrayUniform.size);
          for (int i = 0; i < arrayUniform.size; ++i)
          {
            String uniformName = GetUniformName(arrayUniform.uniform);
            uniformName        = uniformName + "[" + std::to_string(i) + "]";
            GLint loc          = glGetUniformLocation(program->m_handle, uniformName.c_str());
            program->m_defaultArrayUniformLocations[arrayUniform.uniform].push_back(loc);
          }
        }
      }

      m_programs[{vertexShader->m_shaderHandle, fragmentShader->m_shaderHandle}] = program;

      return m_programs[{vertexShader->m_shaderHandle, fragmentShader->m_shaderHandle}];
    }

    return progIter->second;
  }

  void GpuProgramManager::FlushPrograms() { m_programs.clear(); }

} // namespace ToolKit