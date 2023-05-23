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

#include "LightMeshGenerator.h"

#include "EditorLight.h"
#include "Global.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // LightMeshGenerator
    //////////////////////////////////////////////////////////////////////////

    LightMeshGenerator::LightMeshGenerator(Light* light)
    {
      m_targetLight = light;

      m_lightMesh   = std::make_shared<MeshComponent>();
      m_lightMesh->SetCastShadowVal(false);
      m_lightMesh->ParamMesh().m_exposed       = false;
      m_lightMesh->ParamCastShadow().m_exposed = false;
    }

    LightMeshGenerator::~LightMeshGenerator() { m_targetLight = nullptr; }

    void LightMeshGenerator::TransferGizmoMesh(LineBatchRawPtrArray& lines)
    {
      if (lines.empty())
      {
        return;
      }

      LineBatch* lb = lines[0];
      MeshPtr accum = lb->GetMeshComponent()->GetMeshVal();
      lb->GetMeshComponent()->SetMeshVal(nullptr);
      SafeDel(lb);

      for (int i = 1; i < (int) lines.size(); i++)
      {
        LineBatch* lb   = lines[i];
        MeshPtr subMesh = lb->GetMeshComponent()->GetMeshVal();
        accum->m_subMeshes.push_back(subMesh);
        lb->GetMeshComponent()->SetMeshVal(nullptr);
        SafeDel(lb);
      }

      lines.clear();
      m_lightMesh->SetMeshVal(accum);
    }

    // SpotLightMeshGenerator
    //////////////////////////////////////////////////////////////////////////

    SpotLightMeshGenerator::SpotLightMeshGenerator(SpotLight* light) : LightMeshGenerator(light) {}

    void SpotLightMeshGenerator::InitGizmo()
    {
      assert(m_targetLight->GetType() == EntityType::Entity_SpotLight);

      // Middle line.
      SpotLight* sLight = static_cast<SpotLight*>(m_targetLight);

      float r           = sLight->GetRadiusVal();
      Vec3 d            = Vec3(0.0f, 0.0f, -1.0f);

      m_pnts[0]         = Vec3(ZERO);
      m_pnts[1]         = Vec3(d * r * 2.25f);

      m_innerCirclePnts.clear();
      m_innerCirclePnts.reserve(m_circleVertexCount + 1);

      m_outerCirclePnts.clear();
      m_outerCirclePnts.reserve(m_circleVertexCount + 1);

      LineBatchRawPtrArray lines = {new LineBatch(), new LineBatch(), new LineBatch()};

      // Calculating circles.
      int zeroCount              = 0;
      if (d.x == 0.0f)
      {
        zeroCount++;
      }
      if (d.y == 0.0f)
      {
        zeroCount++;
      }
      if (d.z == 0.0f)
      {
        zeroCount++;
      }

      Vec3 per;
      if (zeroCount == 0)
      {
        per.z = 0.0f;
        per.x = -d.y;
        per.y = d.x;
      }
      else if (zeroCount == 1)
      {
        if (d.x == 0.0f)
        {
          per.x = 0.0f;
          per.y = -d.z;
          per.z = d.y;
        }
        else if (d.y == 0.0f)
        {
          per.y = 0.0f;
          per.x = -d.z;
          per.z = d.x;
        }
        else
        {
          per.z = 0.0f;
          per.y = -d.x;
          per.x = d.y;
        }
      }
      else if (zeroCount == 2)
      {
        if (d.x == 0.0f && d.y == 0.0f)
        {
          per.x = 1.0f;
          per.y = 0.0f;
          per.z = 0.0f;
        }
        else if (d.x == 0.0f && d.z == 0.0f)
        {
          per.x = 1.0f;
          per.y = 0.0f;
          per.z = 0.0f;
        }
        else
        {
          per.x = 0.0f;
          per.y = 1.0f;
          per.z = 0.0f;
        }
      }

      per                     = glm::normalize(per);
      d                       = d * r;

      float innerCircleRadius = r * glm::tan(glm::radians(sLight->GetInnerAngleVal() / 2));

      float outerCircleRadius = r * glm::tan(glm::radians(sLight->GetOuterAngleVal() / 2));

      Vec3 inStartPoint       = d + per * innerCircleRadius;
      Vec3 outStartPoint      = d + per * outerCircleRadius;

      m_innerCirclePnts.push_back(inStartPoint);
      m_outerCirclePnts.push_back(outStartPoint);

      float deltaAngle = glm::two_pi<float>() / m_circleVertexCount;

      for (int i = 1; i < m_circleVertexCount + 1; i++)
      {
        // Inner circle vertices.
        m_rot        = glm::rotate(m_identityMatrix, deltaAngle, d);
        inStartPoint = Vec3(m_rot * Vec4(inStartPoint, 1.0f));
        m_innerCirclePnts.push_back(inStartPoint);

        // Outer circle vertices.
        m_rot         = glm::rotate(m_identityMatrix, deltaAngle, d);
        outStartPoint = Vec3(m_rot * Vec4(outStartPoint, 1.0f));
        m_outerCirclePnts.push_back(outStartPoint);
      }

      lines[0]->Generate(m_innerCirclePnts, g_lightGizmoColor, DrawType::LineStrip, 1.0f);

      lines[1]->Generate(m_outerCirclePnts, g_lightGizmoColor, DrawType::LineStrip, 1.0f);

      // Cone
      m_conePnts.resize(2 * (m_circleVertexCount / 4));

      int coneIndex = 0;
      for (int i = 0; i < m_circleVertexCount; i += 4)
      {
        m_conePnts[coneIndex]     = ZERO;
        m_conePnts[coneIndex + 1] = m_outerCirclePnts[i];
        coneIndex                 += 2;
      }

      lines[2]->Generate(m_conePnts, g_lightGizmoColor, DrawType::Line, 1.0f);

      TransferGizmoMesh(lines);
    }

    // DirectionalLightMeshGenerator
    //////////////////////////////////////////////////////////////////////////

    DirectionalLightMeshGenerator::DirectionalLightMeshGenerator(DirectionalLight* light) : LightMeshGenerator(light) {}

    void DirectionalLightMeshGenerator::InitGizmo()
    {
      assert(m_targetLight->GetType() == EntityType::Entity_DirectionalLight);

      // Middle line
      Vec3 d    = Vec3(0.0f, 0.0f, -1.0f);
      Vec3 norm = glm::normalize(d);

      m_pnts.resize(2);
      m_pnts[0] = Vec3(ZERO);
      m_pnts[1] = Vec3(d * 10.0f);

      LineBatchRawPtrArray lines {new LineBatch()};
      lines[0]->Generate(m_pnts, g_lightGizmoColor, DrawType::Line, 1.0f);

      TransferGizmoMesh(lines);
    }

    // PointLightMeshGenerator
    //////////////////////////////////////////////////////////////////////////

    PointLightMeshGenerator::PointLightMeshGenerator(PointLight* light) : LightMeshGenerator(light) {}

    void PointLightMeshGenerator::InitGizmo()
    {
      assert(m_targetLight->GetType() == EntityType::Entity_PointLight);

      m_circlePntsXY.resize(m_circleVertexCount + 1);
      m_circlePntsYZ.resize(m_circleVertexCount + 1);
      m_circlePntsXZ.resize(m_circleVertexCount + 1);

      Vec3 up                    = Vec3(0.0f, 1.0f, 0.0f);
      Vec3 right                 = Vec3(1.0f, 0.0f, 0.0f);
      Vec3 forward               = Vec3(0.0f, 0.0f, 1.0f);
      float deltaAngle           = glm::two_pi<float>() / m_circleVertexCount;

      PointLight* pLight         = static_cast<PointLight*>(m_targetLight);

      Vec3 lightPos              = pLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);

      LineBatchRawPtrArray lines = {new LineBatch(), new LineBatch(), new LineBatch()};

      auto drawCircleGizmoFn     = [&pLight, &deltaAngle, &lines, this](Vec3Array vertices,
                                                                    const Vec3& axis,
                                                                    const Vec3& perpAxis,
                                                                    int lineBatchIndex)
      {
        Vec3 startingPoint  = perpAxis * pLight->GetRadiusVal();
        vertices[0]         = startingPoint;
        Mat4 idendityMatrix = Mat4(1.0f);

        for (int i = 1; i < m_circleVertexCount + 1; ++i)
        {
          Mat4 rot      = glm::rotate(idendityMatrix, deltaAngle, axis);
          startingPoint = Vec3(rot * Vec4(startingPoint, 1.0f));
          vertices[i]   = startingPoint;
        }

        lines[lineBatchIndex]->Generate(vertices, g_lightGizmoColor, DrawType::LineStrip, 1.0f);
      };

      // Circle on XY plane
      drawCircleGizmoFn(m_circlePntsXY, forward, up, 0);

      // Circle on YZ plane
      drawCircleGizmoFn(m_circlePntsXY, right, up, 1);

      // Circle on XZ plane
      drawCircleGizmoFn(m_circlePntsXY, up, right, 2);

      TransferGizmoMesh(lines);
    }

  } // namespace Editor
} // namespace ToolKit