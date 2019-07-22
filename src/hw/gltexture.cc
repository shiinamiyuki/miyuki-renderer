#include <hw/texture.h>
#include <GL/gl3w.h>
#include <utils/log.h>
namespace Miyuki {
	namespace HW {
		static void PrintGLError() {
			auto err = glGetError();
			if (err != GL_NO_ERROR) {
				Log::log("{}\n", err);
			}
		}
		Texture::Texture(size_t width, size_t height)
			:width(width), height(height) {
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			resize(width, height);
		}

		Texture::Texture(size_t width, size_t height, uint8_t* data) : width(width), height(height) {
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		Texture::Texture(const IO::Image& image)
			: width(image.width), height(image.height) {
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			std::vector<unsigned char> pixelBuffer;
			for (const auto& i : image.pixelData) {
				auto out = removeNaNs(i).toInt();
				pixelBuffer.emplace_back(out.r());
				pixelBuffer.emplace_back(out.g());
				pixelBuffer.emplace_back(out.b());
				pixelBuffer.emplace_back(255);
			}
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, &pixelBuffer[0]);
			

		}

		void Texture::resize(size_t width, size_t height) {
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
			this->width = width;
			this->height = height;
		}

		void Texture::setPixel(size_t x, size_t y, const Spectrum& _color) {
			auto color = _color.toInt();
			uint8_t data[8];
			data[0] = color[0];
			data[1] = color[1];
			data[2] = color[2];
			data[3] = 255;
			glTextureSubImage2D(texture, 0, x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		void Texture::setData(uint8_t* data) {
			use();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			PrintGLError();
			//glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		
		void Texture::use() {
			glBindTexture(GL_TEXTURE_2D, texture);
		}
		Texture::~Texture() {
			glDeleteTextures(1, &texture);
		}
	}
}