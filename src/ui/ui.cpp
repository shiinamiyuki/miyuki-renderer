// MIT License
//
// Copyright (c) 2019 椎名深雪
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <api/graph.h>
#include <api/log.hpp>
#include <api/property.hpp>
#include <api/ui/ui.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <api/defs.h>
#include <fstream>
#include <imgui.h>
#include <mutex>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
namespace miyuki::ui {
    void InitializeGLFW() {
        static std::once_flag flag;
        std::call_once(flag, []() {
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit()) {
                MIYUKI_THROW(std::runtime_error, "Cannot initialize GLFW!");
            }
        });
    }
    void InitializeOpenGLLoader() {
        static std::once_flag flag;
        std::call_once(flag, []() {
            bool err = gl3wInit() != 0;
            if (err) {
                MIYUKI_THROW(std::runtime_error, "Cannot initialize OpenGL loader!");
            }
        });
    }

    class AbstractMainWindow::Impl {

      public:
        AbstractMainWindow *mw;
        GLFWwindow *window;
        Impl(int width, int height, const std::string &title) {
            InitializeGLFW();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync
            InitializeOpenGLLoader();

            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Window

            const char *glsl_version = "#version 130";
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(glsl_version);
        }
        void draw() {
            ImGuiIO &io = ImGui::GetIO();
            (void)io;
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                mw->update();

                ImGui::Render();
                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClearColor(0, 0, 0, 0);
                glClear(GL_COLOR_BUFFER_BIT);
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

                if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                    GLFWwindow *backup_current_context = glfwGetCurrentContext();
                    ImGui::UpdatePlatformWindows();
                    ImGui::RenderPlatformWindowsDefault();
                    glfwMakeContextCurrent(backup_current_context);
                }
                glfwSwapBuffers(window);
            }
        }
    };
    AbstractMainWindow::AbstractMainWindow(int width, int height, const std::string &title) {
        impl = new Impl(width, height, title);
        impl->mw = this;
    }
    AbstractMainWindow::~AbstractMainWindow() { delete impl; }
    void AbstractMainWindow::show() { impl->draw(); }

    void SetupDockingSpace(const std::string &name) {
        static bool opt_fullscreen_persistant = true;
        bool opt_fullscreen = opt_fullscreen_persistant;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen) {
            ImGuiViewport *viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the
        // pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(name.c_str(), nullptr, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // DockSpace
        ImGuiIO &io = ImGui::GetIO();

        ImGuiID dockspace_id = ImGui::GetID(name.c_str());
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        ImGui::End();
    }

    class InspectorPropertyVisitor : public PropertyVisitor {
      public:
        // Inherited via PropertyVisitor
        virtual void visit(IntProperty *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::InputInt(prop->name(), &value, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(FloatProperty *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::InputFloat(prop->name(), &value, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(Float3Property *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::InputFloat3(prop->name(), (float *)&value, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(ObjectProperty *prop) override {
            if (ImGui::TreeNodeEx(prop->name(), ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushID(prop->name());

                if (prop->getConstRef()) {
                    ImGui::Text("%s", prop->getConstRef()->getType()->name());
                    prop->getRef()->accept(this);
                }
                ImGui::TreePop();

                ImGui::PopID();
            }
        }
        virtual void visit(FileProperty *prop) override {
            ImGui::PushID(prop->name());
            if (ImGui::Button("select")) {
                auto filename = GetOpenFileNameWithDialog(nullptr);
                if (!filename.empty()) {
                    prop->getRef() = fs::path(filename);
                }
            }
            ImGui::PopID();
        }

        // Inherited via PropertyVisitor
        virtual void visit(Int2Property *) override {}
        virtual void visit(Float2Property *) override {}
    };

    class MainWindow : public AbstractMainWindow {
        std::shared_ptr<core::SceneGraph> graph;
        std::weak_ptr<Object> selected;
        std::function<void(void)> modalFunc = []() {};
        bool _modalOpen = false;
        bool _updated = false;
        template <class F> void showModal(const char *name, F &&f) {
            _modalOpen = true;
            _updated = false;
            modalFunc = [=]() {
                if (!_updated) {
                    ImGui::OpenPopup(name);
                    _updated = true;
                }
                if (_modalOpen) {
                    if (ImGui::BeginPopupModal(name)) {
                        f();
                        ImGui::EndPopup();
                    }
                }
            };
        }
        void closeModal() {
            modalFunc = []() {};
            _modalOpen = false;
        }

        void showSelectable(const std::string &name, const std::shared_ptr<Object> &p) {
            if (ImGui::Selectable(name.c_str(), selected.expired() ? false : p == selected.lock())) {
                selected = p;
            }
        }

        void explore() {
            if (!graph)
                return;
            if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (ImGui::TreeNodeEx("Shapes", ImGuiTreeNodeFlags_DefaultOpen)) {
                    for (auto &i : graph->shapes) {
                        showSelectable(i->toString(), i);
                    }
                    ImGui::TreePop();
                }
                showSelectable("Camera", graph->camera);
                showSelectable("Sampler", graph->sampler);
                showSelectable("Integrator", graph->integrator);
                ImGui::TreePop();
            }
        }
        void showExplorer() {
            if (ImGui::Begin("Explorer")) {
                explore();
                ImGui::End();
            }
        }
        void showInspector() {
            if (ImGui::Begin("Inspector")) {

                ImGui::End();
            }
        }

        void showMenu() {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Open")) {
                        auto filename = GetOpenFileNameWithDialog(nullptr);
                        if (!filename.empty()) {
                            auto path = fs::path(filename);
                            std::ifstream in(filename);
                            serialize::InputArchive ar(in);
                            graph = std::dynamic_pointer_cast<core::SceneGraph>(serialize::ReadObject(ar));
                        }
                    }
                    if (ImGui::MenuItem("Open (json)")) {
                        auto filename = GetOpenFileNameWithDialog(nullptr);
                        if (!filename.empty()) {
                            auto path = fs::path(filename);
                            std::ifstream in(filename);

                            std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                            json data = json::parse(str);
                            graph = std::make_shared<core::SceneGraph>();
                            graph->initialize(data);
                        }
                    }
                    if (ImGui::MenuItem("Save")) {
                    }

                    if (ImGui::MenuItem("Save As")) {
                        if (!graph) {
                            showModal("Error", [=]() {
                                ImGui::Text("%s", "Current scene is empty!");
                                if (ImGui::Button("Close")) {
                                    ImGui::CloseCurrentPopup();
                                }
                            });

                        } else {
                            auto filename = GetSaveFileNameWithDialog(nullptr);
                            if (!filename.empty()) {
                                std::ofstream out(filename);
                                serialize::OutputArchive ar(out);
                                serialize::WriteObject(ar, graph);
                            }
                        }
                    }

                    if (ImGui::MenuItem("Import")) {
                        auto filename = GetOpenFileNameWithDialog(nullptr);
                        if (!filename.empty()) {
                            std::thread th([=]() {
                                showModal("Importing", [=]() {

                                });
                                closeModal();
                            });
                            th.detach();
                        }
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

      public:
        using AbstractMainWindow::AbstractMainWindow;
        void update() override {
            modalFunc();
            SetupDockingSpace("DockingSpace");
            showMenu();
            showExplorer();
            showInspector();
            ImGui::ShowDemoWindow();
        }
    };

    std::shared_ptr<AbstractMainWindow> MakeMainWindow(int width, int height, const std::string &title) {
        return std::make_shared<MainWindow>(width, height, title);
    }
} // namespace miyuki::ui