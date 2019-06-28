#pragma once

// A generic IMGUI interface
// Just a wrapper
#include <miyuki.h>
#include <imgui/imgui.h>
#include <ui/input.h>

namespace Miyuki {
	namespace GUI {
		inline void EmptyFunc() {}
		template<class Derived>
		struct Base {
			Derived& show() {
				This()->showImpl();
				return *This();
			}
			Derived& setActive(bool* active) {
				This()->setActiveImpl();
				return *This();
			}
			Derived& with(bool value, const std::function<void(void)>& handler) {
				if (value) {
					handlerActive = handler;
				}
				else {
					handlerInactive = handler;
				}
				return *This();
			}
			Derived& open(bool* p) {
				_open = p;
				return *This();
			}
			Derived& name(const std::string& name) {
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
			Derived* This() {
				return static_cast<Derived*>(this);
			}
			const Derived* This()const {
				return static_cast<Derived*>(this);
			}
		};
		template<class Derived>
		class Selectable {
		protected:
			bool* _selected = nullptr;
		public:
			Derived& selected(bool* p) {
				_selected = p;
				return *static_cast<Derived*>(this);
			}
		};

		template<class Derived>
		class SingleSelectable {
		protected:
			bool _selected = false;
		public:
			Derived& selected(bool p) {
				_selected = p;
				return *static_cast<Derived*>(this);
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

		class Text : public Base<Text> {
		public:
			void showImpl() {
				ImGui::Text(nameCStr());
			}
		};

		class SelectableText : public Base<SelectableText>, public Selectable<SelectableText> {
		public:
			void showImpl() {
				if (ImGui::Selectable(nameCStr(), _selected)) {
					active();
				}
				else {
					inactive();
				}
			}
		};

		class SingleSelectableText : public Base<SingleSelectableText>, public SingleSelectable<SingleSelectableText> {
		public:
			void showImpl() {
				if (ImGui::Selectable(nameCStr(), _selected)) {
					active();
				}
				else {
					inactive();
				}
			}
		};

		class DoubleClickableText : public Base<DoubleClickableText>, public Selectable<DoubleClickableText> {
		public:
			void showImpl() {
				if (ImGui::Selectable(nameCStr(), _selected, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(0))
						active();
					else
						inactive();
				}
				else {
					inactive();
				}
			}
		};

		class ColorText : public Base<ColorText> {
			Spectrum _color;
		public:
			ColorText& color(const Spectrum& color) {
				_color = color;
				return *this;
			}
			void showImpl() {
				ImGui::TextColored(ImVec4(_color[0], _color[1], _color[2], 1.0f), nameCStr());
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
			int _flags = 0;
		public:
			TreeNode& flag(int flag) {
				_flags |= flag;
				return *this;
			}
			TreeNode& clearFlag() {
				_flags = 0;
				return *this;
			}
			void showImpl() {
				if (ImGui::TreeNodeEx(nameCStr(),_flags)) {
					active();
					ImGui::TreePop();
				}
				else {
					inactive();
				}
			}
		};

		class MenuItem :public  Base<MenuItem>, public Selectable<MenuItem> {
		public:
			void showImpl() {
				if (ImGui::MenuItem(nameCStr(), "", _selected)) {
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
				if (ImGui::BeginMenu(nameCStr(), openFlag() ? *openFlag() : true)) {
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
			Menu& item(const MenuItem& _item) {
				items.emplace_back(_item);
				return *this;
			}
		};
		class MenuBar : public Base<MenuBar> {
		protected:
			std::vector<Menu> _menu;
		public:
			MenuBar& menu(const Menu& bar) {
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
		class MainMenuBar : public Base<MainMenuBar> {
		protected:
			std::vector<Menu> _menu;
		public:
			MainMenuBar& menu(const Menu& bar) {
				_menu.emplace_back(bar);
				return *this;
			}
			void showImpl() {
				if (ImGui::BeginMainMenuBar()) {
					active();
					for (auto& m : _menu) {
						m.show();
					}
					ImGui::EndMainMenuBar();
				}
				else {
					inactive();
				}
			}
		};

		using WindowFlag = int;

		template<class Derived>
		class BaseWindow {
		protected:
			WindowFlag _flag = 0;
		public:
			Derived& flag(WindowFlag f) {
				_flag |= f;
				return *static_cast<Derived*>(this);
			}
			Derived& clearFlag() {
				_flag = 0;
				return *static_cast<Derived*>(this);
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

		class Modal : public Base<Modal>, public BaseWindow<Modal> {
			bool _openModal = false;
			bool _updated = false;
		public:
			Modal& open() {
				_openModal = true;
				_updated = false;
				return *this;
			}

			Modal& close() {
				_openModal = false;
				_updated = false;
				return *this;
			}

			void showImpl() {
				if (_openModal) {
					if (!_updated) {
						ImGui::OpenPopup(nameCStr());
						_updated = true;
					}
					else {

					}
				}				
				ImGui::SetNextWindowSize(ImVec2(400, 200));
				if (ImGui::BeginPopupModal(nameCStr(), openFlag(), _flag)) {
					if(!_openModal) {
						if (!_updated) {
							ImGui::CloseCurrentPopup();
							_updated = true;
						}
					}else
						active();
					ImGui::EndPopup();
				}
				else {
					inactive();
				}
			}
		};
	}
}

