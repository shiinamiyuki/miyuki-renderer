#include <hw/shader.h>
#include <utils/log.h>
namespace Miyuki {
	namespace HW {
		Shader::Shader(const std::string& source, ShaderType type) :type(type) {
			if (type == kVertexShader)
				id = glCreateShader(GL_VERTEX_SHADER);
			else if (type == kFragmentShader)
				id = glCreateShader(GL_FRAGMENT_SHADER);
			else {
				std::exit(-1);
			}
			auto str = source.c_str();
			glShaderSource(id, 1, &str, NULL);
			glCompileShader(id);
			glGetShaderiv(id, GL_COMPILE_STATUS, &success);
			std::vector<char> infoLog(4096);
			if (!success) {
				glGetShaderInfoLog(id, 4096, NULL, &infoLog[0]);
				Log::log("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n{}\n", &infoLog[0]);
			}
		}
		Shader::~Shader() {
			glDeleteShader(id);
		}
		ShaderProgram::ShaderProgram(Shader && vert, Shader && frag) {
			CHECK(vert.type == kVertexShader);
			CHECK(frag.type == kFragmentShader);
			id = glCreateProgram();
			glAttachShader(id, vert.id);
			glAttachShader(id, frag.id);
			glLinkProgram(id);
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			std::vector<char> infoLog(4096);
			if (!success) {
				glGetShaderInfoLog(id, 4096, NULL, &infoLog[0]);
				Log::log("ERROR::SHADER::PROGRAM::LINKING_FAILED\n{}\n", &infoLog[0]);
			}
		}
		void ShaderProgram::use() {
			glUseProgram(id);
		}
		void ShaderProgram::setBool(const std::string & name, bool value) const {
			glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
		}
		void ShaderProgram::setInt(const std::string & name, int value) const {
			glUniform1i(glGetUniformLocation(id, name.c_str()), value);
		}
		void ShaderProgram::setFloat(const std::string & name, float value) const {
			glUniform1f(glGetUniformLocation(id, name.c_str()), value);
		}
		ShaderProgram::~ShaderProgram() {
			glDeleteProgram(id);
		}
	}
}