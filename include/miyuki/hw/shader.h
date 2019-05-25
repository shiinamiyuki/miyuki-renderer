#pragma once
#include <miyuki.h>
#include <utils/noncopyable.hpp>
#include <math/spectrum.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

namespace Miyuki {
	namespace HW {
		enum ShaderType {
			kVertexShader,
			kFragmentShader
		};
		class ShaderProgram;
		class Shader : NonCopyable {
			uint32_t id;
			ShaderType type;
			int success;
		public:
			friend class ShaderProgram;
			Shader() = delete;
			Shader(const std::string& source, ShaderType type);
			bool valid()const { return success; }
			~Shader();
		};
		class ShaderProgram : NonCopyable {
			uint32_t id;
			int success;
		public:
			ShaderProgram(Shader&& vert, Shader&& frag);
			bool valid()const { return success; }
			void use();
			void setBool(const std::string& name, bool value) const;
			void setInt(const std::string& name, int value) const;
			void setFloat(const std::string& name, float value) const;
			~ShaderProgram();
		};
	}
}