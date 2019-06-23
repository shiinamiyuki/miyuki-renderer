#pragma once

// A generic IMGUI interface
// Just a wrapper
#include <miyuki.h>
#include <imgui/imgui.h>
#include <ui/input.h>

namespace Miyuki {
	namespace GUI {
		inline void EmptyFunc() {}
		template<class T>
		struct Base {
			T& show() {
				This()->showImpl();
				return *This();
			}
			T& setActive(bool* active) {
				This()->setActiveImpl();
				return *This();
			}
			T& with(bool value, const std::function<void(void)>& handler) {
				if (value) {
					handlerActive = handler;
				}
				else {
					handlerInactive = handler;
				}
				return *This();
			}
			T& open(bool* p) {
				_open = p;
				return *This();
			}
			T& name(const std::string& name) {
				_name = name;
				return *This();
			}

			const char* nameCStr() {
				return _name.c_str();
			}
			bool* openFlag() { return _open; }
		protected:
			std::string _name;
			bool* _open = nullptr;
			void active() {
				handlerActive();
			}
			void inactive() {
				handlerInactive();
			}
			void setActive(bool value) {
				This()->setActiveImpl(value);
			}
			std::function<void(void)> handlerActive = EmptyFunc, handlerInactive = EmptyFunc;

		private:
			T* This() {
				return (T*)this;
			}
			const T* This()const {
				return (T*)this;
			}
		};
		class Button : public Base<Button> {

		public:
			void showImpl() {
				if (ImGui::Button(nameCStr())) {
					active();
				}
				else {
					inactive();
				}
			}
		};
		class Separator {
		public:
			void show() {
				ImGui::Separator();
			}
		};
		template<typename T>
		class Input : public Base<Input<T>> {
			T* _value;
		public:
			Input& value(T& v) {
				_value = &v;
				return *this;
			}
			void showImpl() {
				if (auto r = GetInput<T>(nameCStr(), *_value)) {
					active();
					*_value = r.value();
				}
				else {
					inactive();
				}
			}
		};

		class TreeNode : public Base<TreeNode> {
		public:
			void showImpl() {
				if (ImGui::TreeNode(nameCStr())) {
					active();
					ImGui::TreePop();
				}
				else {
					inactive();
				}
			}
		};

		class MenuItem :public  Base<MenuItem> {
		public:
			void showImpl() {
				if (ImGui::MenuItem(nameCStr(), "", openFlag())) {
					active();
				}
				else {
					inactive();
				}
			}
		};
		class Menu : public Base<Menu> {
			std::vector<MenuItem> items;
		public:
			void showImpl() {
				if (ImGui::BeginMenu(nameCStr(), openFlag() ? *openFlag(): true)) {
					active();
					for (auto& i : items) {
						i.show();
					}
					ImGui::EndMenu();
				}
				else {
					inactive();
				}
			}
			Menu& item(MenuItem&& _item) {
				items.emplace_back(_item);
				return *this;
			}
		};
		class MenuBar : public Base<MenuBar> {
			std::vector<Menu> _menu;
		public:
			MenuBar& menu(Menu&& bar) {
				_menu.emplace_back(bar);
				return *this;
			}
			void showImpl() {
				if (ImGui::BeginMenuBar()) {
					active();
					for (auto& m : _menu) {
						m.show();
					}
					ImGui::EndMenuBar();
				}
				else {
					inactive();
				}
			}
		};
		using WindowFlag = int;
		template<class T>
		class BaseWindow {
		protected:
			WindowFlag _flag;
		public:
			T& flag(WindowFlag f) {
				_flag |= f;
				return *static_cast<T*>(this);
			}
			T& clearFlag() {
				_flag = 0;
				return *static_cast<T*>(this);
			}
		};
		class Window : public Base<Window>, public BaseWindow<Window> {
		public:
			void showImpl() {
				if (ImGui::Begin(nameCStr(), openFlag(), _flag)) {
					active();
					ImGui::End();
				}
				else {
					inactive();
				}
			}
		};
		class ChildWindow : public Base<ChildWindow>, public BaseWindow<Window> {
		public:
			void showImpl() {
				if (ImGui::BeginChild(nameCStr(), ImVec2(0, 0), false, _flag)) {
					active();
					ImGui::EndChild();
				}
				else {
					inactive();
				}
			}
		};
	}
}

