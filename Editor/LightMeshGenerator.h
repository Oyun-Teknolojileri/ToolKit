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

#pragma once

namespace ToolKit
{
  namespace Editor
  {

    class LightMeshGenerator
    {
     public:
      LightMeshGenerator(Light* light);
      virtual ~LightMeshGenerator();

      virtual void InitGizmo() = 0;

      /**
       * Constructs a MeshComponent from the given lines. Destroys
       * all LineBatch objects in the lines array and clears the array.
       * @param lines is the line batch array that contains generated gizmo
       * data.
       */
      void TransferGizmoMesh(LineBatchRawPtrArray& lines);

     public:
      MeshComponentPtr m_lightMesh; //!< Component that contains generated data.

     protected:
      Light* m_targetLight = nullptr; //!< Light to generate mesh for.
    };

    class SpotLightMeshGenerator : public LightMeshGenerator
    {
     public:
      explicit SpotLightMeshGenerator(SpotLight* light);
      void InitGizmo() override;

     private:
      const int m_circleVertexCount = 36;

      Mat4 m_identityMatrix;
      Mat4 m_rot;

      Vec3 m_pnts[2];
      Vec3Array m_innerCirclePnts;
      Vec3Array m_outerCirclePnts;
      Vec3Array m_conePnts;
    };

    class DirectionalLightMeshGenerator : public LightMeshGenerator
    {
     public:
      explicit DirectionalLightMeshGenerator(DirectionalLight* light);
      void InitGizmo() override;

     private:
      Vec3Array m_pnts;
    };

    class PointLightMeshGenerator : public LightMeshGenerator
    {
     public:
      explicit PointLightMeshGenerator(PointLight* light);
      void InitGizmo() override;

     private:
      const int m_circleVertexCount = 30;

      Vec3Array m_circlePntsXY;
      Vec3Array m_circlePntsYZ;
      Vec3Array m_circlePntsXZ;
    };

  } // namespace Editor
} // namespace ToolKit