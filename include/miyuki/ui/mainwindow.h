#pragma once
#include <miyuki.h>
#include <math/vec.hpp>
#include <utils/log.h>
#include <hw/texture.h>
#include <hw/shader.h>
#include <engine/renderengine.h>
#include <core/film.h>


#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

#include <ui/mykui.h>
#include <ui/uivisitor.h>

namespace Miyuki {
	namespace GUI {
		class MainWindow {
			friend class UIVisitor;
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
			cxx::filesystem::path programPath;
			json config;
			Arc<Core::Film> viewportUpdate;
			std::vector<uint8_t> pixelData;
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
			
			void loadViewImpl();
			void loadView(Arc<Core::Film>);
			struct WindowFlags {
				bool showLog = true;
				bool showPreference = false;
				bool showExplorer = true;
				bool showAttributeEditor = true;
				bool showAbout = false;
				bool showView = true;
				std::atomic<bool> viewportUpdateAvailable;
				WindowFlags() :viewportUpdateAvailable(false) {}
			}windowFlags;

			Modal modal;
			
			UIVisitor visitor;
			void newEngine() {
				engine = std::make_unique<RenderEngine>();
				visitor.reset();
				visitor.engine = engine.get();
			}
			std::optional<Point2i> lastViewportMouseDown;
			Vec3f cameraDir, cameraPos;
			Vec3f center;
			float distance;
		public:
			void showModal() {
				modal.show();
			}
			void closeModal() {
				modal.close();
			}
			void openModal(const std::string& title, std::function<void(void)> f) {
				modal.name(title).open().with(true, f);
			}
			void openModal(std::function<void(void)> f) {
				modal.open().with(true, f);
			}
			void showErrorModal(const std::string& title, const std::string& error);
			MainWindow(int argc, char** argv);
			void show();
			~MainWindow() {}
		};
	}
}