#pragma once
#include <api/math.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// Serves as a thin Object-Oriented wrapper against ImGui library
namespace miyuki::ui {
    class UIObject : public std::enable_shared_from_this<UIObject> {
      protected:
        std::vector<std::shared_ptr<UIObject>> children;
        std::weak_ptr<UIObject> parent;
        bool showed = true;

      public:
        virtual void draw() = 0;
        virtual void add(const std::shared_ptr<UIObject> &child) {
            children.emplace_back(child);
            child->parent = weak_from_this();
        }
        bool isClosed() const { return !showed; }
        void setClosed(bool c) { showed = !c; }
    };

    class Text : public UIObject {
        std::string text;

      public:
        Text(const std::string &text) : text(text) {}

        // Inherited via UIObject
        virtual void draw() override;
    };
    class Modal : public UIObject {
        std::function<void(Modal *)> func;

      public:
        Modal(const std::function<void(Modal *)> &func) : func(func) {}
        void draw() { func(this); }
    };
    class MainWindow : public UIObject {
        class Impl;
        Impl *impl = nullptr;

      public:
        MainWindow(int width, int height, const std::string &title);
        void draw() override;
        void add(const std::shared_ptr<UIObject> &child)override;
        ~MainWindow();
    };

    class Window : public UIObject {
        std::string name;

      public:
        Window(const std::string &name) : name(name) {}
        void draw() override;
    };

    class TreeNode : public UIObject {
        std::string name;

      public:
        TreeNode(const std::string &name) : name(name) {}
        void draw() override;
    };

    class DemoWindow : public UIObject {
      public:
        void draw() override;
    };

    class DockingSpace : public UIObject {
        std::string name;

      public:
        DockingSpace(const std::string &name) : name(name) {}
        void draw() override;
    };

    class Button : public UIObject {
        std::string text;
        std::function<void(void)> callback;

      public:
        Button(const std::string &text) : text(text), callback([]() {}) {}
        void draw() override;
        void setCallback(const std::function<void(void)> &cb) { callback = cb; }
    };

    class CheckBox : public UIObject {
        std::string text;
        std::function<void(bool)> callback;
        bool checked = false;

      public:
        CheckBox(const std::string &text, bool checked = false) : text(text), checked(checked), callback([](bool) {}) {}
        void draw() override;
        void setCallback(const std::function<void(bool)> &cb) { callback = cb; }
    };

    class Slider : public UIObject {
        std::string label;
        std::function<void(float)> callback;
        float min, max;
        float value;

      public:
        Slider(const std::string &label, float value, float min, float max)
            : value(value), min(min), max(max), label(label) {}
        void draw() override;
        void setCallback(const std::function<void(float)> &cb) { callback = cb; }
    };

    std::optional<int> GetInput(const char *, int);
    std::optional<float> GetInput(const char *, float);
    std::optional<Vec3f> GetInput(const char *, const Vec3f &);
    std::optional<std::string> GetInput(const char *, const std::string &);

    template <class T> class Input : public UIObject {
        std::string label;
        T value;
        std::function<void(T)> callback;

      public:
        Input(const std::string &label, T value) : label(label), value(value) {}
        void draw() override {

            if (auto r = GetInput(label.c_str(), &value)) {
                callback(r.value());
            }
        }
        void setCallback(const std::function<void(T)> &cb) { callback = cb; }
    };

} // namespace miyuki::ui