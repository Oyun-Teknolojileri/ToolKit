/*
/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  /**
   * Class that holds programs for the gpu's programmable pipeline stages. A program consists of
   * shaders for stages. Vertex and Fragmant stages are supported.
   */
  class TK_API GpuProgram
  {
   public:
    GpuProgram();
    GpuProgram(ShaderPtr vertex, ShaderPtr fragment);
    ~GpuProgram();

   public:
    uint m_handle = 0;
    String m_tag;
    ShaderPtrArray m_shaders;
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