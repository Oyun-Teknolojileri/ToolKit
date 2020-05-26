#pragma once

#include "Types.h"
#include <variant>

namespace ToolKit
{

	class ParameterVariant
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

		ParameterVariant() { SetVar(0); }
		ParameterVariant(float var) { SetVar(var); }
		ParameterVariant(int var) { SetVar(var); }
		ParameterVariant(const Vec3& var) { SetVar(var); }
		ParameterVariant(const Vec4& var) { SetVar(var); }
		ParameterVariant(const Mat3& var) { SetVar(var); }
		ParameterVariant(const Mat4& var) { SetVar(var); }

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

}