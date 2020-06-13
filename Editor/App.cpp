#include "stdafx.h"

#include "App.h"
#include "Renderer.h"
#include "UI.h"
#include "Viewport.h"
#include "Primative.h"
#include "Node.h"
#include "GlobalDef.h"
#include "OverlayUI.h"
#include "Grid.h"
#include "Directional.h"
#include "Mod.h"
#include "ConsoleWindow.h"
#include "Gizmo.h"
#include "FolderWindow.h"
#include "DebugNew.h"

#include <filesystem>
#include <cstdlib>

namespace ToolKit
{
	namespace Editor
	{

		App::App(int windowWidth, int windowHeight)
		{
			m_suzanne = nullptr;
			m_knight = nullptr;
			m_knightRunAnim = nullptr;
			m_q1 = nullptr;
			m_q2 = nullptr;
			m_q3 = nullptr;
			m_q4 = nullptr;
			m_cursor = nullptr;
			m_lightMaster = nullptr;
			m_renderer = new Renderer();
			m_renderer->m_windowWidth = windowWidth;
			m_renderer->m_windowHeight = windowHeight;
		}

		App::~App()
		{
			Destroy();

			// Engine components.
			SafeDel(m_renderer);
		}

		void App::Init()
		{
			m_suzanne = new Drawable();
			m_suzanne->m_node->SetTranslation({ 0.0f, 0.0f, -5.0f });
			m_suzanne->m_node->SetOrientation(glm::angleAxis(-glm::half_pi<float>(), X_AXIS));
			Mesh* szm = GetMeshManager()->CreateDerived<Mesh>(MeshPath("suzanne.mesh"))->GetCopy();
			szm->Init(false);
			m_suzanne->m_mesh = MeshPtr(szm);
			m_scene.AddEntity(m_suzanne);

			// https://t-allen-studios.itch.io/low-poly-saxon-warrior
			m_knight = new Drawable();
			m_knight->m_mesh = GetSkinMeshManager()->Create(MeshPath("Knight.skinMesh"));
			m_knight->m_node->SetScale({ 0.01f, 0.01f, 0.01f });
			m_knight->m_node->SetTranslation({ 0.0f, 0.0f, 5.0f });
			m_scene.AddEntity(m_knight);

			m_knightRunAnim = GetAnimationManager()->Create(AnimationPath("Knight_Armature_Run.anim"));
			m_knightRunAnim->m_loop = true;
			GetAnimationPlayer()->AddRecord(m_knight, m_knightRunAnim.get());

			m_cursor = new Cursor();

			MaterialPtr normalMat = GetMaterialManager()->Create(MaterialPath("objectNormal.material"));

			m_q1 = new Cube();
			m_q1->m_mesh->m_material = normalMat;
			m_q1->m_mesh->Init(false);
			m_q1->m_node->SetTranslation({ 2.0f, 0.0f, 0.0f });
			m_q1->m_node->Rotate(glm::angleAxis(glm::half_pi<float>(), Y_AXIS), TransformationSpace::TS_LOCAL);
			m_q1->m_node->Rotate(glm::angleAxis(glm::half_pi<float>(), Z_AXIS), TransformationSpace::TS_LOCAL);
			m_scene.AddEntity(m_q1);

			m_q2 = new Cube();
			m_q2->m_mesh->m_material = normalMat;
			m_q2->m_mesh->Init(false);
			m_q2->m_node->SetTranslation({ 2.0f, 0.0f, 2.0f });
			m_q2->m_node->SetOrientation(glm::angleAxis(glm::half_pi<float>(), Y_AXIS));
			m_scene.AddEntity(m_q2);

			m_q3 = new Cone({ 1.0f, 1.0f, 30, 30 });
			m_q3->m_mesh->m_material = normalMat;
			m_q3->m_mesh->Init(false);
			m_q3->m_node->Scale({ 0.3f, 1.0f, 0.3f });
			m_q3->m_node->SetTranslation({ 2.0f, 0.0f, 0.0f });
			m_scene.AddEntity(m_q3);

			m_q1->m_node->AddChild(m_q2->m_node);
			m_q2->m_node->AddChild(m_q3->m_node);

			m_q4 = new Cube();
			m_q4->m_mesh->m_material = normalMat;
			m_q4->m_mesh->Init(false);
			m_q4->m_node->SetTranslation({ 4.0f, 0.0f, 0.0f });
			m_scene.AddEntity(m_q4);

			m_origin = new Axis3d();

			m_grid = new Grid(100);
			m_grid->m_mesh->Init(false);

			MaterialPtr solidColorMaterial = GetMaterialManager()->GetCopyOfSolidMaterial();
			m_highLightMaterial = MaterialPtr(solidColorMaterial->GetCopy());
			m_highLightMaterial->m_color = g_selectHighLightPrimaryColor;
			m_highLightMaterial->GetRenderState()->cullMode = CullingType::Front;

			m_highLightSecondaryMaterial = MaterialPtr(solidColorMaterial->GetCopy());
			m_highLightSecondaryMaterial->m_color = g_selectHighLightSecondaryColor;
			m_highLightSecondaryMaterial->GetRenderState()->cullMode = CullingType::Front;

			ModManager::GetInstance()->Init();
			ModManager::GetInstance()->SetMod(true, ModId::Select);
			ActionManager::GetInstance()->Init();

			// Lights and camera.
			m_lightMaster = new Node();

			Light* light = new Light();
			light->Yaw(glm::radians(-45.0f));
			m_lightMaster->AddChild(light->m_node);
			m_sceneLights.push_back(light);

			light = new Light();
			light->m_intensity = 0.5f;
			light->Yaw(glm::radians(60.0f));
			m_lightMaster->AddChild(light->m_node);
			m_sceneLights.push_back(light);

			light = new Light();
			light->m_intensity = 0.3f;
			light->Yaw(glm::radians(-140.0f));
			m_lightMaster->AddChild(light->m_node);
			m_sceneLights.push_back(light);

			// UI.
			// Perspective.
			Viewport* vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
			vp->m_name = "Perspective";
			vp->m_camera->m_node->SetTranslation({ 5.0f, 3.0f, 5.0f });
			vp->m_camera->Pitch(glm::radians(-20.0f));
			vp->m_camera->RotateOnUpVector(glm::radians(30.0f));
			m_windows.push_back(vp);

			// Orthographic.
			vp = new Viewport(m_renderer->m_windowWidth * 0.8f, m_renderer->m_windowHeight * 0.8f);
			vp->m_name = "Orthographic";
			vp->m_camera->m_node->SetTranslation({ 0.0f, 500.0f, 0.0f });
			vp->m_camera->Pitch(glm::radians(-90.0f));
			vp->m_cameraAlignment = 1;
			vp->m_orthographic = true;
			m_windows.push_back(vp);

			ConsoleWindow* console = new ConsoleWindow();
			m_windows.push_back(console);

			FolderWindow* assetBrowser = new FolderWindow();
			assetBrowser->m_name = g_assetBrowserStr;
			assetBrowser->Iterate(ResourcePath());
			m_windows.push_back(assetBrowser);

			UI::InitIcons();
		}

		void App::Destroy()
		{
			// UI.
			for (Window* wnd : m_windows)
			{
				SafeDel(wnd);
			}
			m_windows.clear();

			SafeDel(Viewport::m_overlayMods);
			SafeDel(Viewport::m_overlayOptions);

			m_scene.Destroy();

			// Editor objects.
			SafeDel(m_grid);
			SafeDel(m_origin);
			SafeDel(m_cursor);
			SafeDel(m_lightMaster);
			for (int i = 0; i < 3; i++)
			{
				SafeDel(m_sceneLights[i]);
			}
			assert(m_sceneLights.size() == 3);
			m_sceneLights.clear();

			GetAnimationPlayer()->m_records.clear();

			ModManager::GetInstance()->UnInit();
			ActionManager::GetInstance()->UnInit();
		}

		void App::Frame(float deltaTime)
		{
			// Update animations.
			GetAnimationPlayer()->Update(MilisecToSec(deltaTime));

			// Update Viewports.
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() != Window::Type::Viewport)
				{
					continue;
				}

				UI::DispatchSignals(wnd);

				Viewport* vp = static_cast<Viewport*> (wnd);
				vp->Update(deltaTime);

				// Adjust scene lights.
				Camera* cam = vp->m_camera;
				m_lightMaster->OrphanSelf();
				cam->m_node->AddChild(m_lightMaster);

				m_renderer->SetRenderTarget(vp->m_viewportImage);

				for (Entity* ntt : m_scene.GetEntities())
				{
					if (ntt->IsDrawable())
					{
						if (ntt->GetType() == EntityType::Entity_Billboard)
						{
							Billboard* billboard = static_cast<Billboard*> (ntt);
							billboard->LookAt(cam, vp->m_height);
						}

						m_renderer->Render(static_cast<Drawable*> (ntt), cam, m_sceneLights);
					}
				}

				RenderSelected(vp);

				if (!m_perFrameDebugObjects.empty())
				{
					for (Drawable* d : m_perFrameDebugObjects)
					{
						m_renderer->Render(d, cam);
						SafeDel(d);
					}
					m_perFrameDebugObjects.clear();
				}

				// Scale grid spacing.
				if (cam->IsOrtographic())
				{
					Vec3 pos = cam->m_node->GetTranslation(TransformationSpace::TS_WORLD);
					float dist = glm::distance(Vec3(), pos);
					float scale = glm::max(1.0f, glm::trunc(dist / 100.0f));
					m_grid->Resize(500, 1.0f / scale);
				}
				else
				{
					m_grid->Resize(500, 1.0f);
				}
				m_renderer->Render(m_grid, cam);

				m_origin->LookAt(cam, vp->m_height);
				m_renderer->Render(m_origin, cam);

				// Only draw gizmo in active viewport.
				if (m_gizmo != nullptr && vp->IsActive())
				{
					m_gizmo->LookAt(cam, vp->m_height);
					glClear(GL_DEPTH_BUFFER_BIT);
					if (PolarGizmo* pg = dynamic_cast<PolarGizmo*> (m_gizmo))
					{
						pg->Render(m_renderer, cam);
					}
					else
					{
						m_renderer->Render(m_gizmo, cam);
					}
				}

				m_cursor->LookAt(cam, vp->m_height);
				m_renderer->Render(m_cursor, cam);
			}

			m_renderer->SetRenderTarget(nullptr);

			// Render UI.
			UI::ShowUI();

			// Update Mods.
			ModManager::GetInstance()->Update(deltaTime);
		}

		void App::OnResize(int width, int height)
		{
			m_renderer->m_windowWidth = width;
			m_renderer->m_windowHeight = height;
			glViewport(0, 0, width, height);
		}

		void App::OnNewScene(const String& name)
		{
			Destroy();
			Init();
			g_app->m_scene.m_name = name;
		}

		void App::OnSaveScene()
		{

		}

		void App::OnQuit()
		{
			static bool processing = false;
			if (!processing)
			{
				YesNoWindow* reallyQuit = new YesNoWindow("Quiting... Are you sure?##ClsApp");
				reallyQuit->m_yesCallback = []()
				{
					g_running = false;
				};

				reallyQuit->m_noCallback = []()
				{
					processing = false;
				};

				UI::m_volatileWindows.push_back(reallyQuit);
				processing = true;
			}
		}

		int App::Import(const String& fullPath, const String& subDir, bool overwrite)
		{
			if (!CanImport(fullPath))
			{
				if (ConsoleWindow* con = GetConsole())
				{
					con->AddLog("Import failed: " + fullPath, ConsoleWindow::LogType::Error);
					con->AddLog("File format is not supported.\nSuported formats are fbx, glb, obj.", ConsoleWindow::LogType::Error);
				}
				return -1;
			}

			std::filesystem::path pathBck = std::filesystem::current_path();
			if (CheckFile(fullPath))
			{
				// Set the execute path.
				std::filesystem::path path = pathBck.u8string() + "\\..\\Utils\\Import";
				std::filesystem::current_path(path);

				std::filesystem::path cpyDir = ".";
				if (!subDir.empty())
				{
					cpyDir += '\\' + subDir;
					std::filesystem::create_directories(cpyDir);
				}

				std::filesystem::copy
				(
					fullPath, cpyDir,
					overwrite ? std::filesystem::copy_options::overwrite_existing
					: std::filesystem::copy_options::skip_existing
				);

				String name, ext;
				DecomposePath(fullPath, nullptr, &name, &ext);

				String cmd = "Import \"";
				if (!subDir.empty())
				{
					cmd += subDir + '\\' + name + ext;
				}
				else
				{
					cmd += name + ext;
				}

				cmd += "\" -s " + std::to_string(UI::ImportData.scale);

				// Execute command.
				int result = std::system(cmd.c_str());
				assert(result != -1);

				// Move assets.
				String meshFile;
				if (result != -1)
				{
					std::ifstream copyList("out.txt");
					if (copyList.is_open())
					{
						// Check files.
						StringArray missingFiles;
						for (String line; std::getline(copyList, line); )
						{
							if (!CheckFile(line))
							{
								missingFiles.push_back(line);
							}
						}

						if (!missingFiles.empty())
						{
							if (g_app->m_importSlient)
							{
								g_app->GetConsole()->AddLog("Import: " + fullPath + " failed.", ConsoleWindow::LogType::Error);
								goto Fail;
							}

							// Try search.
							size_t numFound = 0;
							for (String& searchPath : UI::SearchFileData.searchPaths)
							{
								for (String& missingFile : missingFiles)
								{
									String name, ext;
									DecomposePath(missingFile, nullptr, &name, &ext);
									String missingFullPath = searchPath + '\\' + name + ext;
									if (CheckFile(missingFullPath))
									{
										numFound++;
										std::filesystem::copy
										(
											missingFullPath, cpyDir,
											std::filesystem::copy_options::overwrite_existing
										);
									}
								}
							}

							if (numFound < missingFiles.size())
							{
								// Retry.
								UI::SearchFileData.missingFiles = missingFiles;
								goto Retry;
							}
						}

						copyList.clear();
						copyList.seekg(0, std::ios::beg);
						for (String line; std::getline(copyList, line); )
						{
							String ext;
							DecomposePath(line, nullptr, nullptr, &ext);

							String fullPath;
							if (ext == MESH || ext == SKINMESH)
							{
								fullPath = MeshPath(line);
								meshFile = fullPath;
							}

							if (ext == SKELETON)
							{
								fullPath = SkeletonPath(line);
							}

							if (ext == ANIM)
							{
								fullPath = AnimationPath(line);
							}

							if 
								(
									ext == PNG ||
									ext == JPEG ||
									ext == TGA ||
									ext == BMP ||
									ext == PSD
								)
							{
								fullPath = TexturePath(line);
							}

							if (ext == MATERIAL)
							{
								fullPath = MaterialPath(line);
							}

							fullPath = "..\\" + fullPath; // Resource dir is one more level up.

							String path, name;
							DecomposePath(fullPath, &path, &name, &ext);
							std::filesystem::create_directories(path);
							std::filesystem::copy
							(
								line, fullPath,
								std::filesystem::copy_options::overwrite_existing
							);
						}
					}
				}
				else
				{
					goto Fail;
				}

				std::filesystem::current_path(pathBck);
				if (!meshFile.empty())
				{
					String ext;
					DecomposePath(meshFile, nullptr, nullptr, &ext);
					MeshPtr mesh;
					if (ext == SKINMESH)
					{
						mesh = GetSkinMeshManager()->Create(meshFile);
					}
					else
					{
						mesh = GetMeshManager()->Create(meshFile);
					}

					if (FolderWindow* browser = GetAssetBrowser())
					{
						browser->UpdateContent();
					}
				}

				UI::SearchFileData.showSearchFileWindow = false;
				return result;
			}
			else
			{
				goto Fail;
			}

		Retry:
			UI::SearchFileData.showSearchFileWindow = true;

		Fail:
			std::filesystem::current_path(pathBck);
			return -1;
		}

		bool App::CanImport(const String& fullPath)
		{
			String ext;
			DecomposePath(fullPath, nullptr, nullptr, &ext);

			if (ext == ".fbx")
			{
				return true;
			}

			if (ext == ".glb")
			{
				return true;
			}

			if (ext == ".obj")
			{
				return true;
			}

			return false;
		}

		Viewport* App::GetActiveViewport()
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() != Window::Type::Viewport)
				{
					continue;
				}

				if (wnd->IsActive() && wnd->IsVisible())
				{
					return static_cast<Viewport*> (wnd);
				}
			}

			return nullptr;
		}

		Viewport* App::GetViewport(const String& name)
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->m_name == name)
				{
					return dynamic_cast<Viewport*> (wnd);
				}
			}

			return nullptr;
		}

		ConsoleWindow* App::GetConsole()
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() == Window::Type::Console)
				{
					return static_cast<ConsoleWindow*> (wnd);
				}
			}

			return nullptr;
		}

		FolderWindow* App::GetAssetBrowser()
		{
			for (Window* wnd : m_windows)
			{
				if (wnd->GetType() == Window::Type::Browser)
				{
					if (wnd->m_name == g_assetBrowserStr)
					{
						return static_cast<FolderWindow*> (wnd);
					}
				}
			}

			return nullptr;
		}

		void App::RenderSelected(Viewport* vp)
		{
			if (m_scene.GetSelectedEntityCount() == 0)
			{
				return;
			}

			auto RenderFn = [this, vp](const EntityRawPtrArray& selection, const Vec3& color)
			{
				glEnable(GL_STENCIL_TEST);
				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);
				glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

				RenderTarget stencilMask((int)vp->m_width, (int)vp->m_height);
				stencilMask.Init();

				m_renderer->SetRenderTarget(&stencilMask);

				for (Entity* ntt : selection)
				{
					if (ntt->IsDrawable())
					{
						m_renderer->Render(static_cast<Drawable*> (ntt), vp->m_camera);
					}
				}

				glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
				ShaderPtr solidColor = GetShaderManager()->Create(ShaderPath("solidColorFrag.shader"));
				m_renderer->DrawFullQuad(solidColor);
				glDisable(GL_STENCIL_TEST);

				m_renderer->SetRenderTarget(vp->m_viewportImage, false);

				// Dilate.
				glBindTexture(GL_TEXTURE_2D, stencilMask.m_textureId);
				ShaderPtr dilate = GetShaderManager()->Create(ShaderPath("dilateFrag.shader"));
				dilate->SetShaderParameter("Color", color);
				m_renderer->DrawFullQuad(dilate);
			};
			
			EntityRawPtrArray selecteds;
			m_scene.GetSelectedEntities(selecteds);
			Entity* primary = selecteds.back();

			selecteds.pop_back();
			RenderFn(selecteds, g_selectHighLightSecondaryColor);
			
			selecteds.clear();
			selecteds.push_back(primary);
			RenderFn(selecteds, g_selectHighLightPrimaryColor);
		}

	}
}
