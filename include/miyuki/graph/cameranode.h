#pragma once

#include <graph/graphnode.h>

namespace Miyuki {
	namespace Graph {
		class CameraNode : public GraphNode {
		public:
			MYK_CLASS(CameraNode, GraphNode);
		};

		class PerspectiveCameraNode : public CameraNode {
			MYK_CLASS(PerspectiveCameraNode, CameraNode);
		};
	}
}