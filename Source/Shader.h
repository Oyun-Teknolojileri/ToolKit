#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include <variant>

namespace ToolKit
{

  enum class Uniform
  {
    PROJECT_MODEL_VIEW,
    MODEL,
    INV_TR_MODEL,
    LIGHT_DATA,
    CAM_DATA,
    COLOR,
    FRAME_COUNT
  };

  class ShaderVariant
  {
  public:
    enum class VariantType
    {
      Float,
      Int,
      Vec3,
      Vec4,
      Mat3,
      Mat4
    };

    ShaderVariant() { SetVar(0); }
    ShaderVariant(float var) { SetVar(var); }
    ShaderVariant(int var) { SetVar(var); }
    ShaderVariant(const Vec3& var) { SetVar(var); }
    ShaderVariant(const Vec4& var) { SetVar(var); }
    ShaderVariant(const Mat3& var) { SetVar(var); }
    ShaderVariant(const Mat4& var) { SetVar(var); }

    VariantType GetType() { return m_type; }
    template<typename T> T GetVar() { return std::get<T>(m_var); }
    void SetVar(float var) { m_type = VariantType::Float; m_var = var; }
    void SetVar(int var) { m_type = VariantType::Int; m_var = var; }
    void SetVar(const Vec3& var) { m_type = VariantType::Vec3; m_var = var; }
    void SetVar(const Vec4& var) { m_type = VariantType::Vec4; m_var = var; }
    void SetVar(const Mat3& var) { m_type = VariantType::Mat3; m_var = var; }
    void SetVar(const Mat4& var) { m_type = VariantType::Mat4; m_var = var; }

  private:
    std::variant<float, int, Vec3, Vec4, Mat3, Mat4> m_var;
    VariantType m_type;
  };

  class Shader : public Resource
  {
  public:
    Shader();
    Shader(String file);
    ~Shader();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;
    void SetShaderParameter(String param, const ShaderVariant& val);

  public:
    std::unordered_map<String, ShaderVariant> m_shaderParams;

    GLuint m_type = GL_VERTEX_SHADER;
    GLuint m_shaderHandle = 0;
    std::vector<Uniform> m_uniforms;
    String m_source;
  };

	class Program
	{
	public:
		Program();
		Program(ShaderPtr vertex, ShaderPtr fragment);
		~Program();

	public:
		GLuint m_handle = 0;
		String m_tag;
		ShaderPtrArray m_shaders;
	};

  class ShaderManager : public ResourceManager<Shader>
  {
  public:
    void Init();
  };

}