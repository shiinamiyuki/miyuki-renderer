#pragma once

#include <graph/graphcompiler.h>
#include <reflection.h>
namespace Miyuki {
	namespace Graph {
		class MeshNode: public Reflection::Object {
		public:
			MYK_CLASS(MeshNode, Reflection::Object);
		};
	}
}