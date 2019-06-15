#ifndef MIYUKI_SHADER_NODE_H
#define MIYUKI_SHADER_NODE_H

#include <graph/graphnode.h>


namespace Miyuki {
	namespace Graph {
		class ShaderNode : public GraphNode {
		public:
			MYK_CLASS(ShaderNode, GraphNode);
			virtual Float evalFloat() {
				throw NotImplemented();
			}
			virtual Vec3f evalFloat3() {
				throw NotImplemented();
			}
		};
		class FloatNode : public ShaderNode {
		public:
			MYK_CLASS(FloatNode, ShaderNode);
			virtual Float evalFloat() {
				return value->getValue();
			}
			virtual Vec3f evalFloat3() {
				return Vec3f(value->getValue());
			}
		protected:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Float, value);
			MYK_END_PROPERTY;
		};


		class RGBNode : public ShaderNode {
		public:
			MYK_CLASS(RGBNode, ShaderNode);
			virtual Float evalFloat() {
				throw NotImplemented();
			}
			virtual Vec3f evalFloat3() {
				throw value->getValue();
			}
		protected:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(Vec3f, value);
			MYK_END_PROPERTY;
		};

		class TextureNode : public ShaderNode {
		public:
			MYK_CLASS(TextureNode, ShaderNode);
		};

		class ImageTextureNode : public TextureNode {
		public:
			MYK_CLASS(ImageTextureNode, TextureNode);
		protected:
			MYK_BEGIN_PROPERTY;
			MYK_PROPERTY(File, file);
			MYK_END_PROPERTY;
		};
	}
}
#endif