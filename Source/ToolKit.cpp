#include "stdafx.h"
#include "ToolKit.h"
#include "Logger.h"
#include "DebugNew.h"

namespace ToolKit
{

	Main Main::m_instance;

	Main::~Main()
	{
	}

	void Main::Init()
	{
		Logger::GetInstance()->Log("ToolKit Initialization");
		m_animationMan.Init();
		m_textureMan.Init();
		m_meshMan.Init();
		m_skinMeshMan.Init();
		m_spriteSheetMan.Init();
		m_audioMan.Init();
		m_shaderMan.Init();
		m_materialManager.Init();

		m_initiated = true;
	}

	void Main::Uninit()
	{
		m_animationPlayer.m_records.clear();
		m_animationMan.Uninit();
		m_textureMan.Uninit();
		m_meshMan.Uninit();
		m_skinMeshMan.Uninit();
		m_spriteSheetMan.Uninit();
		m_audioMan.Uninit();
		m_shaderMan.Uninit();
		m_materialManager.Uninit();

		m_initiated = false;
	}

	Main::Main()
	{
	}

	Main* Main::GetInstance()
	{
		return &m_instance;
	}

	AnimationManager* GetAnimationManager()
	{
		return &Main::GetInstance()->m_animationMan;
	}

	AnimationPlayer* GetAnimationPlayer()
	{
		return &Main::GetInstance()->m_animationPlayer;
	}

	AudioManager* GetAudioManager()
	{
		return &Main::GetInstance()->m_audioMan;
	}

	MaterialManager* GetMaterialManager()
	{
		return &Main::GetInstance()->m_materialManager;
	}

	MeshManager* GetMeshManager()
	{
		return &Main::GetInstance()->m_meshMan;
	}

	ShaderManager* GetShaderManager()
	{
		return &Main::GetInstance()->m_shaderMan;
	}

	SkinMeshManager* GetSkinMeshManager()
	{
		return &Main::GetInstance()->m_skinMeshMan;
	}

	SpriteSheetManager* GetSpriteSheetManager()
	{
		return &Main::GetInstance()->m_spriteSheetMan;
	}

	TextureManager* GetTextureManager()
	{
		return &Main::GetInstance()->m_textureMan;
	}

	String TexturePath(String file)
	{
		String path = "../Resources/Textures/";
		path += file;
		return path;
	}

	String MeshPath(String file)
	{
		String path = "../Resources/Meshes/";
		path += file;
		return path;
	}

	String FontPath(String file)
	{
		String path = "../Resources/Fonts/";
		path += file;
		return path;
	}

	String SpritePath(String file)
	{
		String path = "../Resources/Sprites/";
		path += file;
		return path;
	}

	String AudioPath(String file)
	{
		String path = "../Resources/Audio/";
		path += file;
		return path;
	}

	String AnimationPath(String file)
	{
		String path = "../Resources/Meshes/";
		path += file;
		return path;
	}

	String SkeletonPath(String file)
	{
		String path = "../Resources/Meshes/";
		path += file;
		return path;
	}

	String ShaderPath(String file)
	{
		String path = "../Resources/Shaders/";
		path += file;
		return path;
	}

	String MaterialPath(String file)
	{
		String path = "../Resources/Materials/";
		path += file;
		return path;
	}

}
