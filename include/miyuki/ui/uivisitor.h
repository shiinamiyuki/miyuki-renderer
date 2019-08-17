#pragma once


#include <engine/renderengine.h>
#include <imgui/imgui.h>
#include <boost/signals2.hpp>

namespace Miyuki {
	namespace GUI {
		class MainWindow;
		class UIVisitor : public Reflection::ReflectionVisitor {
			Reflective* selected = nullptr;

			void visitMaterialAndSelect(Box<Core::Material>& material, const std::string& label);
			void visitShaderAndSelect(Box<Core::Shader>& shader, const std::string& label);
			enum SelectedNodeType {
				kMaterial,
				kMesh,
				kObject
			};
			int selectedMaterialIndex = -1;
			SelectedNodeType selectedNodeType;
			using Base = Reflection::ReflectionVisitor;
			boost::signals2::connection connection;
			bool changed = true;
			MainWindow& window;
			void loadWindowView(Arc<Core::Film>&);
			std::mutex renderCBMutex;
		public:
			void commit() {
				if(changed)
					engine->commit();
				changed = false;
			}
	
			RenderEngine* engine = nullptr;
			void visitGraph();
			void init();
			UIVisitor(MainWindow & window):window(window) {
				init();
			}
			void reset() {
				changed = true;
				selected = nullptr;
			}
			template<class T>
			void visit(T* node) {
				ImGui::PushID(node);
				ReflectionVisitor::visit(node);
				ImGui::PopID();
			}
			template<class T>
			void visit(Box<T>& node) {
				visit(node.get());
			}
			void visitSelected();
			void visitCamera();
			void visitIntegrator();
			void visitFilm();
			void visitWorld();
			void startInteractive();
			void stopRender();
			~UIVisitor();
		};
	}
}