/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

    String tag = GenerateTag((int) vertexShader->GetIdVal(), (int) fragmentShader->GetIdVal());
    if (m_programs.find(tag) == m_programs.end())
    {
      GpuProgramPtr program = MakeNewPtr<GpuProgram>(vertexShader, fragmentShader);
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

    return m_programs[tag];
  }

  void GpuProgramManager::FlushPrograms() { m_programs.clear(); }

  String GpuProgramManager::GenerateTag(int vertexShaderId, int fragmentShaderId)
  {
    return std::to_string(vertexShaderId) + "|" + std::to_string(fragmentShaderId);
  }

} // namespace ToolKit