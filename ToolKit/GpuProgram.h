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
   * shaders for stages. Vertex and Fragment stages are supported.
   */
  class TK_API GpuProgram
  {
    friend class GpuProgramManager;
    friend class Renderer;

   public:
    GpuProgram();
    GpuProgram(ShaderPtr vertex, ShaderPtr fragment);
    ~GpuProgram();

    /**
     * If caller gives index (different than -1), this function tries to get uniform location as array.
     * Returns -1 if the uniform is not in the program.
     * If the uniform is not registered but is in the program, this function generates the location of the uniform
     */
    int GetUniformLocation(const String& uniformName, int index = -1);

    /**
     * Updates or adds the given uniform to the uniform cache of the program.
     */
    void UpdateUniform(const String& name, const UniformValue& val);
    void UpdateUniform(const ShaderUniform& uniform);

   public:
    uint m_handle = 0;
    ShaderPtrArray m_shaders;
    ULongID m_activeMaterialID = 0;

   private:
    std::unordered_map<String, ShaderUniform> m_uniformsNEW;         // TODO remove "NEW"
    std::unordered_map<String, int> m_uniformLocationsNEW;           // TODO remove "NEW"
    std::unordered_map<String, IntArray> m_arrayUniformLocationsNEW; // TODO remove "NEW"
  };

  constexpr int TKGpuPipelineStages = 2; //!< Number of programmable pipeline stages.

  /**
   * Class that generates the programs from the given shaders and maintains the generated programs.
   */
  class TK_API GpuProgramManager
  {
   public:
    ~GpuProgramManager();

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
    const GpuProgramPtr& CreateProgram(const ShaderPtr& vertexShader, const ShaderPtr& fragmentShader);

    /**
     * Clears all the created programs, effectively forcing renderer to recreate the programs at next run.
     * Intended usage is to clear all the programs develop by additional modules to prevent memory issues
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
