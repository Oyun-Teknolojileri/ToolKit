/*
/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Shader.h"
#include "Types.h"

namespace ToolKit
{

  /**
   * Class that holds programs for the gpu's programmable pipeline stages. A program consists of
   * shaders for stages. Vertex and Fragmant stages are supported.
   */
  class TK_API GpuProgram
  {
    friend class GpuProgramManager;

   public:
    GpuProgram();
    GpuProgram(ShaderPtr vertex, ShaderPtr fragment);
    ~GpuProgram();

    /**
     * If caller gives index (different than -1), this function tries to get uniform location as array.
     * Returns -1 if the uniform location is not registered.
     */
    inline int GetUniformLocation(Uniform uniform, int index = -1)
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
          std::vector<int>& locs = m_arrayUniformLocations[uniform];
          if (locs.size() > index)
          {
            return locs[index];
          }
        }

        return -1;
      }
    }

    int GetShaderParamUniformLoc(const String& uniformName);

   public:
    uint m_handle = 0;
    String m_tag;
    ShaderPtrArray m_shaders;

   private:
    std::unordered_map<Uniform, int> m_uniformLocations;
    std::unordered_map<Uniform, std::vector<int>> m_arrayUniformLocations;
    std::unordered_map<String, int> m_shaderParamsUniformLocations;
  };

  /**
   * Class that generates the programs from the given shaders and maintains the generated programs.
   */
  class TK_API GpuProgramManager
  {
   public:
    /**
     * Creates a gpu program that can be binded to renderer to render the objects with.
     * @param vertexShader - is the vertex shader to use in pipeline.
     * @param fragmentShader - is the fragment program to use in pipeline.
     */
    GpuProgramPtr CreateProgram(ShaderPtr vertexShader, ShaderPtr fragmentShader);

    /**
     * Clears all the created programs, effectively forcing renderer to recreate the programs at next run.
     * Intendent usage is to clear all the programs develop by additional modules to prevent memory issues
     * when the modules are destroyed but the editor still running.
     */
    void FlushPrograms();

    /**
     * Creates a tag for the given shaders. This can be used query existing programs for given shader combination.
     */
    String GenerateTag(int vertexShaderId, int fragmentShaderId);

   private:
    /**
     * Links the given shaders with the program.
     */
    void LinkProgram(uint program, uint vertexShaderId, uint fragmentShaderId);

   private:
    /**
     * Associative array that holds all the programs. Programs are constructed by concatanating their ids together.
     * So this prevents creating same program for the given shaders if one already exist.
     */
    std::unordered_map<String, GpuProgramPtr> m_programs;
  };
} // namespace ToolKit
