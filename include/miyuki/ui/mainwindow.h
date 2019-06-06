#pragma once
#include <miyuki.h>
#include <math/vec.hpp>
#include <utils/log.h>
#include <hw/texture.h>
#include <hw/shader.h>
#include <graph/graph.h>
#include <engine/renderengine.h>

#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

namespace Miyuki {
	namespace GUI {
		class MainWindow {
			//Timer updateTimer;
			std::mutex viewportMutex;
			uint32_t vbo = -1;
			std::unique_ptr<RenderEngine> engine;
			GLFWwindow* window = nullptr;
			void mainLoop();
			void close();
			std::unique_ptr<HW::Texture> viewport;
			std::unique_ptr<HW::Texture> background;
			std::unique_ptr<HW::ShaderProgram> backgroundShader;
			std::unique_ptr<std::thread> renderThread;
			cxx::filesystem::path programPath;
			json config;
			void loadBackGroundShader();
			void loadBackgroundImage();
			void drawBackground();
			void viewportWindow();
			void explorerWindow();;
			void attriubuteEditorWindow();
			void menuBar();
			void logWindow();
			void update();
			void loadConfig();
			void saveConfig();
			void newGraph();
			//void loadViewport(Film&);
			struct WindowFlags{
				bool showLog = true;
				bool showPreference = false;
				bool showExplorer = true;
				bool showAttributeEditor = true;
				bool showAbout = false;
				std::atomic<bool> viewportUpdateAvailable;
				WindowFlags() :viewportUpdateAvailable(false){}
			}windowFlags;
			void startRenderThread();
			void stopRenderThread();
			Graph::Graph* graph() {
				return engine ? engine->graph() : nullptr;
			}
		public:
			MainWindow(int argc, char** argv);
			void show();
			~MainWindow() {}
		};
	}
}