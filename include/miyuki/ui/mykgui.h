#pragma once
#include <miyuki.h>

#include <GLFW/glfw3.h>

namespace Miyuki {
	namespace GUI {
		class Window{
		protected:
			std::string _title;
			void update();
			GLFWwindow* window;
			Spectrum clearColor;
		public:
			Window(const char * title);
			void show();
		};
	}
}
