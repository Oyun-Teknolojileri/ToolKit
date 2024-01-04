/*
/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Shader.h"
#include "ShaderUniform.h"
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
    int GetUniformLocation(Uniform uniform, int index = -1) const;

    /**
     * Try to retrieve uniform location in the program. If its not in the cache, gets it via graphic api and cache its
     * location.
     * @param uniformName is the name that can be found in the shader.
     * @return location of the uniform in the program.
     */
    int GetShaderParamUniformLoc(const String& uniformName);

   public:
    uint m_handle = 0;
    ShaderPtrArray m_shaders;

   private:
    std::unordered_map<Uniform, int> m_uniformLocations;
    std::unordered_map<Uniform, IntArray> m_arrayUniformLocations;
    std::unordered_map<String, int> m_shaderParamsUniformLocations;
  };

  constexpr int TKGpuPipelineStages = 2; //!< Number of programmable pipeline stages.

  /**
   * Class that generates the programs from the given shaders and maintains the generated programs.
   */
  class TK_API GpuProgramManager
  {
   private:
    /**
     * Utility class that hash an array of ULongIDs. Purpose of this class is to provide a hash generator from the
     * shader ids that is used in the program. This hash value will be identifying the program.
     */
    struct IDArrayHash
    {
      std::size_t operator()(const std::array<ULongID, TKGpuPipelineStages>& data) const
      {
        std::size_t hashValue = 0;

        for (const auto& element : data)
        {
          // Combine the hash value with the hash of each element
          hashValue ^= std::hash<ULongID>()(element) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        }

        return hashValue;
      }
    };

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

   private:
    /**
     * Links the given shaders with the program.
     */
    void LinkProgram(uint program, uint vertexShaderId, uint fragmentShaderId);

   private:
    /**
     * Associative array that holds all the programs.
     */
    std::unordered_map<std::array<ULongID, TKGpuPipelineStages>, GpuProgramPtr, IDArrayHash> m_programs;
  };

} // namespace ToolKit
