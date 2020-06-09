#include "stdafx.h"

#include "Viewport.h"
#include "Directional.h"
#include "Renderer.h"
#include "App.h"
#include "GlobalDef.h"
#include "SDL.h"
#include "OverlayUI.h"
#include "Node.h"
#include "Primative.h"
#include "Grid.h"
#include "FolderWindow.h"
#include "ConsoleWindow.h"
#include "Gizmo.h"
#include "DebugNew.h"

namespace ToolKit
{
	namespace Editor
	{

		uint Viewport::m_nextId = 1;
		OverlayMods* Viewport::m_overlayMods = nullptr;
		OverlayViewportOptions* Viewport::m_overlayOptions = nullptr;

		Viewport::Viewport(float width, float height)
			: m_width(width), m_height(height)
		{
			m_camera = new Camera();
			m_camera->SetLens(glm::quarter_pi<float>(), width, height);
			m_viewportImage = new RenderTarget((uint)width, (uint)height);
			m_viewportImage->Init();
			m_name = g_viewportStr + " " + std::to_string(m_nextId++);

			if (m_overlayMods == nullptr)
			{
				m_overlayMods = new OverlayMods(this);
			}

			if (m_overlayOptions == nullptr)
			{
				m_overlayOptions = new OverlayViewportOptions(this);
			}
		}

		Viewport::~Viewport()
		{
			SafeDel(m_camera);
			SafeDel(m_viewportImage);
		}

		void Viewport::Show()
		{
			ImGui::SetNextWindowSize(ImVec2(m_width, m_height), ImGuiCond_Once);
			if (ImGui::Begin(m_name.c_str(), &m_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				HandleStates();

				// Content area size
				ImVec2 vMin = ImGui::GetWindowContentRegionMin();
				ImVec2 vMax = ImGui::GetWindowContentRegionMax();

				vMin.x += ImGui::GetWindowPos().x;
				vMin.y += ImGui::GetWindowPos().y;
				vMax.x += ImGui::GetWindowPos().x;
				vMax.y += ImGui::GetWindowPos().y;

				m_wndPos.x = vMin.x;
				m_wndPos.y = vMin.y;

				m_wndContentAreaSize = Vec2(glm::abs(vMax.x - vMin.x), glm::abs(vMax.y - vMin.y));

				ImGuiIO& io = ImGui::GetIO();
				ImVec2 absMousePos = io.MousePos;
				m_mouseOverContentArea = false;
				if (vMin.x < absMousePos.x && vMax.x > absMousePos.x)
				{
					if (vMin.y < absMousePos.y && vMax.y > absMousePos.y)
					{
						m_mouseOverContentArea = true;
					}
				}

				m_lastMousePosRelContentArea.x = (int)(absMousePos.x - vMin.x);
				m_lastMousePosRelContentArea.y = (int)(absMousePos.y - vMin.y);

				if (!ImGui::IsWindowCollapsed())
				{
					if (m_wndContentAreaSize.x > 0 && m_wndContentAreaSize.y > 0)
					{
						ImGui::Image((void*)(intptr_t)m_viewportImage->m_textureId, ImVec2(m_width, m_height), ImVec2(0.0f, 0.0f), ImVec2(1.0f, -1.0f));

						ImVec2 currSize = ImGui::GetContentRegionMax();
						if (m_wndContentAreaSize.x != m_width || m_wndContentAreaSize.y != m_height)
						{
							OnResize(m_wndContentAreaSize.x, m_wndContentAreaSize.y);
						}

						if (IsActive())
						{
							ImGui::GetWindowDrawList()->AddRect(vMin, vMax, IM_COL32(255, 255, 0, 255));
						}
						else
						{
							ImGui::GetWindowDrawList()->AddRect(vMin, vMax, IM_COL32(128, 128, 128, 255));
						}
					}
				}

				if (g_app->m_showOverlayUI)
				{
					if (IsActive() || g_app->m_showOverlayUIAlways)
					{
						if (m_overlayMods != nullptr)
						{
							m_overlayMods->m_owner = this;
							m_overlayMods->Show();
						}

						if (m_overlayOptions != nullptr)
						{
							m_overlayOptions->m_owner = this;
							m_overlayOptions->Show();
						}
					}
				}

				m_mouseHover = ImGui::IsWindowHovered();

				ImVec2 pos = GLM2IMVEC(m_wndPos);
				pos.x += m_width - 70.0f;
				pos.y += m_wndContentAreaSize.y - 20.0f;
				String fps = "Fps: " + std::to_string(g_app->m_fps);
				ImGui::GetWindowDrawList()->AddText(pos, IM_COL32(255, 255, 0, 255), fps.c_str());

				// Process draw commands.
				ImDrawList* drawList = ImGui::GetWindowDrawList();
				for (auto command : m_drawCommands)
				{
					command(drawList);
				}
				m_drawCommands.clear();

				// AssetBrowser drop handling.
				ImGui::Dummy(GLM2IMVEC(m_wndContentAreaSize));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BrowserDragZone"))
					{
						IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
						DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;

						if (entry.m_ext == MESH)
						{
							String path = entry.m_rootPath + "\\" + entry.m_fileName + entry.m_ext;
							Drawable* dwMesh = new Drawable();
							dwMesh->m_mesh = GetMeshManager()->Create(path);
							dwMesh->m_mesh->Init(false);
							Ray ray = RayFromMousePosition();
							Vec3 pos = PointOnRay(ray, 5.0f);
							g_app->m_grid->HitTest(ray, pos);
							dwMesh->m_node->SetTranslation(pos);
							g_app->m_scene.AddEntity(dwMesh);
							g_app->m_scene.AddToSelection(dwMesh->m_id, false);
							SetActive();
						}
					}
					ImGui::EndDragDropTarget();
				}
			}
			ImGui::End();
		}

		void Viewport::Update(float deltaTime)
		{
			if (!IsActive())
			{
				return;
			}

			// Update viewport mods.
			FpsNavigationMode(deltaTime);
			OrbitPanMod(deltaTime);
		}

		void Viewport::OnResize(float width, float height)
		{
			m_width = width;
			m_height = height;
			if (m_orthographic)
			{
				m_camera->SetLens(width / height, -10.0f, 10.0f, -10.0f, 10.0f, 0.01f, 1000.0f);
			}
			else
			{
				m_camera->SetLens(m_camera->GetData().fov, width, height);
			}

			m_viewportImage->UnInit();
			m_viewportImage->m_width = (uint)width;
			m_viewportImage->m_height = (uint)height;
			m_viewportImage->Init();
		}

		Window::Type Viewport::GetType() const
		{
			return Type::Viewport;
		}

		bool Viewport::IsViewportQueriable()
		{
			return m_mouseOverContentArea && m_mouseHover && m_active && m_visible && m_relMouseModBegin;
		}

		Ray Viewport::RayFromMousePosition()
		{
			return RayFromScreenSpacePoint(GetLastMousePosScreenSpace());
		}

		Ray Viewport::RayFromScreenSpacePoint(const Vec2& pnt)
		{
			Vec2 mcInVs = TransformScreenToViewportSpace(pnt);

			Ray ray;
			ray.position = TransformViewportToWorldSpace(mcInVs);
			if (m_camera->IsOrtographic())
			{
				ray.direction = m_camera->GetDir();
			}
			else
			{
				ray.direction = glm::normalize(ray.position - m_camera->m_node->GetTranslation(TransformationSpace::TS_WORLD));
			}

			return ray;
		}

		Vec3 Viewport::GetLastMousePosWorldSpace()
		{
			return TransformViewportToWorldSpace(GetLastMousePosViewportSpace());
		}

		Vec2 Viewport::GetLastMousePosViewportSpace()
		{
			Vec2 screenPoint = m_lastMousePosRelContentArea;
			screenPoint.y = m_wndContentAreaSize.y - screenPoint.y; // Imgui Window origin Top - Left to OpenGL window origin Bottom - Left

			return screenPoint;
		}

		Vec2 Viewport::GetLastMousePosScreenSpace()
		{
			Vec2 screenPoint = GetLastMousePosViewportSpace();
			screenPoint.y = m_wndContentAreaSize.y - screenPoint.y; // Bring it back from opengl (BottomLeft) to Imgui (TopLeft) system.

			return m_wndPos + screenPoint; // Move it from window space to screen space.
		}

		Vec3 Viewport::TransformViewportToWorldSpace(const Vec2& pnt)
		{
			Vec3 screenPoint = Vec3(pnt, 0.0f);

			Mat4 view = m_camera->GetViewMatrix();
			Mat4 project = m_camera->GetData().projection;

			return glm::unProject(screenPoint, view, project, Vec4(0.0f, 0.0f, m_width, m_height));
		}

		Vec2 Viewport::TransformScreenToViewportSpace(const Vec2& pnt)
		{
			Vec2 vp = pnt - m_wndPos; // In window space.
			vp.y = m_wndContentAreaSize.y - vp.y; // In viewport space.
			return vp;
		}

		void Viewport::FpsNavigationMode(float deltaTime)
		{
			if (m_camera)
			{
				// Mouse is right clicked
				if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
				{
					// Handle relative mouse hack.
					if (m_relMouseModBegin)
					{
						m_relMouseModBegin = false;
						SDL_GetGlobalMouseState(&m_mousePosBegin.x, &m_mousePosBegin.y);
					}

					ImGui::SetMouseCursor(ImGuiMouseCursor_None);
					glm::ivec2 mp;
					SDL_GetGlobalMouseState(&mp.x, &mp.y);
					mp = mp - m_mousePosBegin;
					SDL_WarpMouseGlobal(m_mousePosBegin.x, m_mousePosBegin.y);
					// End of relative mouse hack.

					m_camera->Pitch(-glm::radians(mp.y * g_app->m_mouseSensitivity));
					m_camera->RotateOnUpVector(-glm::radians(mp.x * g_app->m_mouseSensitivity));

					Vec3 dir, up, right;
					dir = -Z_AXIS;
					up = Y_AXIS;
					right = X_AXIS;

					float speed = g_app->m_camSpeed;

					Vec3 move;
					ImGuiIO& io = ImGui::GetIO();
					if (io.KeysDown[SDL_SCANCODE_A])
					{
						move += -right;
					}

					if (io.KeysDown[SDL_SCANCODE_D])
					{
						move += right;
					}

					if (io.KeysDown[SDL_SCANCODE_W])
					{
						move += dir;
					}

					if (io.KeysDown[SDL_SCANCODE_S])
					{
						move += -dir;
					}

					if (io.KeysDown[SDL_SCANCODE_PAGEUP])
					{
						move += up;
					}

					if (io.KeysDown[SDL_SCANCODE_PAGEDOWN])
					{
						move += -up;
					}

					float displace = speed * MilisecToSec(deltaTime);
					if (length(move) > 0.0f)
					{
						move = normalize(move);
					}

					if (m_camera->IsOrtographic())
					{
						Vec3 farPos = -m_camera->GetDir() * 500.0f;
						// For zoom in & out adjust projection size.
						m_camera->m_node->SetTranslation(farPos, TransformationSpace::TS_WORLD);
					}
					else
					{
						m_camera->Translate(move * displace);
					}
				}
				else
				{
					if (!m_relMouseModBegin)
					{
						m_relMouseModBegin = true;
					}
				}
			}
		}

		void Viewport::OrbitPanMod(float deltaTime)
		{
			if (m_camera)
			{
				// Adjust zoom always.
				if (m_mouseOverContentArea)
				{
					float zoom = ImGui::GetIO().MouseWheel;
					m_camera->Translate(Vec3(0.0f, 0.0f, -zoom));
				}

				static bool hitFound = false;
				static Vec3 orbitPnt;
				if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
				{
					// Figure out orbiting point.
					if (!hitFound)
					{
						Ray orbitRay = RayFromMousePosition();
						Scene::PickData pd = g_app->m_scene.PickObject(orbitRay);

						if (pd.entity == nullptr)
						{
							if (!g_app->m_grid->HitTest(orbitRay, orbitPnt))
							{
								orbitPnt = PointOnRay(orbitRay, 5.0f);
							}
						}
						else
						{
							orbitPnt = pd.pickPos;
						}
						hitFound = true;
					}

					// Orbit around it.
					float x = ImGui::GetIO().MouseDelta.x;
					float y = ImGui::GetIO().MouseDelta.y;
					Vec3 r = m_camera->GetRight();
					Vec3 u = m_camera->GetUp();

					if (ImGui::GetIO().KeyShift)
					{
						Vec3 sum = r * -x + u * y;

						Camera::CamData dat = m_camera->GetData();
						float dist = glm::distance(orbitPnt, dat.pos);
						float speed = 0.1f * glm::sqrt(dist);
						float displace = speed * MilisecToSec(deltaTime);
						m_camera->m_node->Translate(sum * displace, TransformationSpace::TS_WORLD);
					}
					else
					{
						Mat4 camTs = m_camera->m_node->GetTransform(TransformationSpace::TS_WORLD);
						Mat4 ts = glm::translate(Mat4(), orbitPnt);
						Mat4 its = glm::translate(Mat4(), -orbitPnt);
						Quaternion qx = glm::angleAxis(-glm::radians(y * g_app->m_mouseSensitivity), r);
						Quaternion qy = glm::angleAxis(-glm::radians(x * g_app->m_mouseSensitivity), Y_AXIS);

						camTs = ts * glm::toMat4(qy * qx) * its * camTs;
						m_camera->m_node->SetTransform(camTs, TransformationSpace::TS_WORLD);
					}
				}

				if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle))
				{
					hitFound = false;
				}
			}
		}

	}
}
