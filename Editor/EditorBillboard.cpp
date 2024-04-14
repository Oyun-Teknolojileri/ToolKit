/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "EditorBillboard.h"

#include <Mesh.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(EditorBillboardBase, Billboard);

    EditorBillboardBase::EditorBillboardBase() {}

    EditorBillboardBase::EditorBillboardBase(const Settings& settings) : Billboard(settings) {}

    void EditorBillboardBase::NativeConstruct()
    {
      Super::NativeConstruct();
      Generate();
    }

    void EditorBillboardBase::Generate()
    {
      MeshComponentPtr mCom = GetComponent<MeshComponent>();

      // Billboard
      QuadPtr quad          = MakeNewPtr<Quad>();
      MeshPtr meshPtr       = quad->GetMeshComponent()->GetMeshVal();
      MaterialPtr matPtr    = GetMaterialManager()->GetCopyOfUnlitMaterial();
      matPtr->UnInit();
      matPtr->m_diffuseTexture                = m_iconImage;
      matPtr->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
      meshPtr->m_material                     = matPtr;
      mCom->SetMeshVal(meshPtr);
    }

    TKDefineClass(SkyBillboard, EditorBillboardBase);

    SkyBillboard::SkyBillboard() : EditorBillboardBase({true, 3.5f, 10.0f}) {}

    SkyBillboard::~SkyBillboard() {}

    EditorBillboardBase::BillboardType SkyBillboard::GetBillboardType() const { return BillboardType::Sky; }

    void FlipNode(Node* node)
    {
      Vec3 scl = node->GetScale();
      if (scl.y > 0.0f)
      {
        node->Scale(Vec3(1.0f, -1.0f, 1.0f));
      }
    }

    void SkyBillboard::LookAt(CameraPtr cam, float scale)
    {
      Super::LookAt(cam, scale);
      FlipNode(m_node);
    }

    void SkyBillboard::Generate()
    {
      m_iconImage = UI::m_skyIcon;
      EditorBillboardBase::Generate();
    }

    TKDefineClass(LightBillboard, EditorBillboardBase);

    LightBillboard::LightBillboard() : EditorBillboardBase({true, 3.5f, 10.0f}) {}

    LightBillboard::~LightBillboard() {}

    EditorBillboardBase::BillboardType LightBillboard::GetBillboardType() const { return BillboardType::Light; }

    void LightBillboard::LookAt(CameraPtr cam, float scale)
    {
      Super::LookAt(cam, scale);
      FlipNode(m_node);
    }

    void LightBillboard::Generate()
    {
      m_iconImage = UI::m_lightIcon;
      EditorBillboardBase::Generate();
    }

  } // namespace Editor
} // namespace ToolKit