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
			void visitMediumAndSelect(Box<Core::Medium>& shader, const std::string& label);
			enum SelectedNodeType {
				kMaterial,
				kMesh,
				kObject
			};
			int selectedMaterialIndex = -1;
			SelectedNodeType selectedNodeType;
			using Base = Reflection::ReflectionVisitor;
			boost::signals2::connection connection;
			size_t modificationCounter = 0;
			MainWindow& window;
			void loadWindowView(Arc<Core::Film>&);
			std::mutex renderCBMutex;
		public:
			void commit() {
				if (hasAnyChanged())
					engine->commit();
				resetChanges();
			}
			bool hasAnyChanged() {
				return modificationCounter != 0;
			}
			void resetChanges() {
				modificationCounter = 0;
			}
			RenderEngine* engine = nullptr;
			void visitGraph();
			void init();
			UIVisitor(MainWindow& window) :window(window) {
				init();
			}
			void reset() {
				modificationCounter = 1;
				selected = nullptr;
			}
			template<class T>
			void visit(T* node) {
				ImGui::PushID(node);
				auto last = modificationCounter;
				ReflectionVisitor::visit(node);
				if (auto r = dynamic_cast<CachedPreprocessable*>(node)) {
					if (last != modificationCounter)
						r->notifyChange();
				}
				if (auto r = dynamic_cast<Core::Light*>(node)) {
					if (last != modificationCounter)
						engine->getGraph()->lights->notifyChange();
				}
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