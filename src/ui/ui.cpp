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

#include <miyuki.renderer/graph.h>
#include <miyuki.foundation/log.hpp>
#include <miyuki.renderer/mesh-importer.h>
#include <miyuki.foundation/property.hpp>
#include <miyuki.renderer/ui/ui.h>
#include <miyuki.renderer/ui/myk-ui.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <miyuki.foundation/defs.h>
#include <fstream>
#include <imgui.h>
#include <mutex>
#include <thread>

#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <miyuki.foundation/film.h>

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void GLAPIENTRY
MessageCallback(GLenum source,
                GLenum type,
                GLuint id,
                GLenum severity,
                GLsizei length,
                const GLchar *message,
                const void *userParam) {
    if (GL_DEBUG_SEVERITY_HIGH == severity)
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                type, severity, message);
}

void SetupDockingSpace(const std::string &name);

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

            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(MessageCallback, nullptr);
        });
    }

    class AbstractMainWindow::Impl {

    public:
        AbstractMainWindow *mw{};
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
            (void) io;
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
            (void) io;
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


    struct ImplSelector {
        std::unordered_map<std::string, Type *> _map;
        std::unordered_map<Type *, std::string> _invmap;
        std::vector<std::string> _list;

        void loadImpl(const std::string &interface) {
            ForeachImplementation(interface, [=](const std::string &impl) {
                auto type = GetType(impl);;
                _map[impl] = type;
                _invmap[type] = impl;
                _list.emplace_back(impl);
            });
        }

        template<class T>
        std::optional<std::shared_ptr<T>> select(const std::string &label, const std::shared_ptr<T> &current) {
            auto itemName = !current ? "Empty" : _invmap.at(current->getType());
            std::optional<std::shared_ptr<T>> opt = std::nullopt;
            Combo().name(label).item(itemName).with(true, [=, &opt]() {
                SingleSelectableText().name("Empty").selected(current == nullptr).with(true, [=, &opt]() {
                    if (current) {
                        opt = {nullptr};
                    }
                    if (!current) {
                        ImGui::SetItemDefaultFocus();
                    }
                }).show();

                for (const auto &ty : _list) {
                    auto currentTy = current ? current->getType() : nullptr;
                    bool is_selected = _map.at(ty) == currentTy;
                    SingleSelectableText().name(ty).selected(is_selected).with(true, [=, &opt]() {
                        if (currentTy != _map.at(ty)) {
                            opt = std::dynamic_pointer_cast<T>(_map.at(ty)->create());
                        }
                    }).show();
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
            }).show();
            return opt;
        }
    };


    template<class T>
    std::optional<std::shared_ptr<T>>
    selectImpl(const std::shared_ptr<T> &p, const std::string &interface, const std::string &label) {
        static ImplSelector selector;
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            selector.loadImpl(interface);
        });
        return selector.select<T>(label, p);
    }


    class InspectorPropertyVisitor : public PropertyVisitor {
    public:
        // Inherited via PropertyVisitor

        virtual void visit(BoolProperty *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::Checkbox(prop->name(), &value)) {
                prop->getRef() = value;
            }
        }

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
            if (ImGui::InputFloat3(prop->name(), (float *) &value, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(RGBProperty *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::ColorPicker3(prop->name(), (float *) &value, ImGuiColorEditFlags_DisplayRGB |
                                                                    ImGuiColorEditFlags_DisplayHSV |
                                                                    ImGuiColorEditFlags_DisplayHex)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(ObjectProperty *prop) override {
            ImGui::PushID(prop->getRef().get());
            if (ImGui::TreeNodeEx(prop->name(), ImGuiTreeNodeFlags_DefaultOpen)) {

                if (prop->type->isInterface()) {
                    ImplSelector selector;
                    selector.loadImpl(prop->type->name());
                    if (auto opt = selector.select(prop->name(), prop->getRef())) {
                        prop->getRef() = opt.value();
                    }
                }
                if (prop->getConstRef()) {
                    ImGui::Text("%s", prop->getConstRef()->getType()->name());
                    prop->getRef()->accept(this);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
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
        virtual void visit(Int2Property *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::InputInt2(prop->name(), (int *) &value[0], ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }

        virtual void visit(Float2Property *prop) override {
            auto value = prop->getConstRef();
            if (ImGui::InputFloat2(prop->name(), (float *) &value, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
                prop->getRef() = value;
            }
        }
    };

    struct Viewport {
        const ivec2 computeDispatchSize = ivec2(16, 16);
        mpsc::channel_t<std::shared_ptr<core::Film>> channel = mpsc::channel<std::shared_ptr<core::Film>>();
        GLuint color = -1;
        GLuint weight = -1;
        GLuint composed = -1;
        ivec2 dim;

        GLuint program;

        void compileShader() {
            GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
            const char *source = R"(
#version 430
layout(local_size_x = 16, local_size_y = 16,local_size_z = 1) in;
layout(binding = 1, rgba32f)  readonly uniform image2D color;
layout(binding = 2, rgba32f)  readonly uniform image2D weight;
layout(binding = 3, rgba32f)  writeonly uniform image2D composedImage;

uniform vec2 iResolution;

void main(){
    if(any(greaterThanEqual(gl_GlobalInvocationID.xy, iResolution)))
        return;
    ivec2 pixelCoord = ivec2(gl_GlobalInvocationID.xy);
    vec3 c = imageLoad(color, pixelCoord).rgb;
    float w = imageLoad(weight, pixelCoord).r;
    if(w == 0.0) w =1.0;
    c = c / w;
    c = pow(c, vec3(1.0/2.2));
    imageStore(composedImage, pixelCoord, vec4(c, 1.0f));
}
)";
            GLint success;
            std::vector<char> error(4096);
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, error.size(), nullptr, error.data());
                std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << error.data() << std::endl;
                exit(1);
            };

            program = glCreateProgram();
            glAttachShader(program, shader);
            glLinkProgram(program);
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(program, error.size(), nullptr, error.data());
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << error.data() << std::endl;
                exit(1);
            }
            glDeleteShader(shader);
        }

        void genTexture() {
            auto setup = [=](GLuint &texture) {
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, dim[0], dim[1], 0, GL_RGBA,
                             GL_FLOAT, NULL);
            };

            setup(color);
            setup(weight);
            setup(composed);
        }

        void writeData(const core::Film &film) {
            if (ivec2(film.width, film.height) != dim) {
                resize(ivec2(film.width, film.height));
            }
            glBindTexture(GL_TEXTURE_2D, color);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim[0], dim[1], GL_RGBA,
                            GL_FLOAT, film.pixels.color.data());

            glBindTexture(GL_TEXTURE_2D, weight);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim[0], dim[1], GL_RGBA,
                            GL_FLOAT, film.pixels.weight.data());
        }

        explicit Viewport(ivec2 dim) : dim(dim), channel(mpsc::channel<std::shared_ptr<core::Film>>()) {
            genTexture();

        }

        void resize(ivec2 _dim) {
            dim = _dim;
            glDeleteTextures(1, &color);
            glDeleteTextures(1, &weight);
            glDeleteTextures(1, &composed);
            genTexture();
        }

        ~Viewport() {
            glDeleteTextures(1, &color);
            glDeleteTextures(1, &weight);
            glDeleteTextures(1, &composed);
        }

        void show() {
            if (!channel.rx.block()) {

                if (auto p = channel.rx.recv()) {
                    writeData(*p.value());
                }
            }
            glUseProgram(program);
            glBindTexture(GL_TEXTURE_2D, color);
            glUniform2f(glGetUniformLocation(program, "iResolution"), dim[0], dim[1]);
            glBindImageTexture(1, color, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(2, weight, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(3, composed, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
            glDispatchCompute(std::ceil(dim[0] / float(computeDispatchSize[0])),
                              std::ceil(dim[1] / float(computeDispatchSize[1])), 1);
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            glFinish();
            ImGui::Image(reinterpret_cast<void *> (composed), ImVec2(dim[0], dim[1]));
        }
    };

    class MainWindow : public AbstractMainWindow {
    public:
        std::shared_ptr<core::SceneGraph> graph;
        std::weak_ptr<Object> selected;
        std::function<void(void)> modalFunc = []() {};
        std::mutex _modalMutex;
        bool _modalOpen = false;
        bool _updated = false;
        fs::path sceneFilePath;
        std::shared_ptr<Viewport> viewport;
        std::optional<Task<core::RenderOutput>> renderTask;

        template<class F>
        void showModal(const char *name, F &&f) {
            std::lock_guard<std::mutex> guard(_modalMutex);
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

        template<class F>
        void showSelectableTreeNode(const std::string &name, const std::shared_ptr<Object> &p, const F &f) {
            int flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (selected.expired() ? false : p == selected.lock()) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (ImGui::TreeNodeEx(name.c_str(), flags)) {
                if (ImGui::IsItemClicked()) {
                    selected = p;
                }
                f();
                ImGui::TreePop();
            }
        }

        void explore() {
            if (!graph)
                return;
            if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
                for (auto &mesh: graph->shapes) {
                    if (auto m = std::dynamic_pointer_cast<core::Mesh>(mesh)) {
                        showSelectableTreeNode(m->toString(), m, [=]() {
                            for (auto &kv:m->materials) {
                                showSelectable(kv.first, kv.second);
                            }
                        });
                    }
                }
            }
        }


        void showView() {
            if (ImGui::Begin("View", nullptr, ImGuiWindowFlags_NoScrollWithMouse)) {
                if (graph) {
                    bool checked = renderTask.has_value();
                    if (ImGui::Checkbox("Render", &checked)) {
                        if (!checked) {
                            if (renderTask.has_value()) {
                                renderTask.value().kill();
                                renderTask = std::nullopt;
                            }
                        } else {
                            if (renderTask.has_value()) {
                                renderTask.value().wait();
                                renderTask = std::nullopt;
                            }
                            if (!graph->integrator) {
                                checked = false;
                            } else {
                                CurrentPathGuard _guard;
                                fs::current_path(fs::path(sceneFilePath).parent_path());
                                renderTask = std::move(graph->createRenderTask(viewport->channel.tx));
                                renderTask.value().launch();
                            }
                        }
                    }

                    if (graph->filmDimension != viewport->dim) {
                        viewport->resize(graph->filmDimension);
                    }
                    viewport->show();
                }
                ImGui::End();
            }
        }

        void showExplorer() {
            if (ImGui::Begin("Explorer")) {
                explore();
                ImGui::End();
            }
        }

        void showCamera() {
            if (ImGui::BeginTabItem("Camera")) {
                if (auto r = selectImpl<core::Camera>(graph->camera, "Camera", "Camera##")) {
                    graph->camera = r.value();
                }
                if (graph->camera) {
                    InspectorPropertyVisitor visitor;
                    graph->camera->accept(&visitor);
                }
                ImGui::EndTabItem();
            }
        }

        void showSampler(){
            if(ImGui::BeginTabItem("Sampler")){
                if(auto r = selectImpl<core::Sampler>(graph->sampler, "Sampler","Sampler##")){
                    graph->sampler = r.value();
                }
                if(graph->sampler){
                    InspectorPropertyVisitor visitor;
                    graph->sampler->accept(&visitor);
                }

            }
        }
        void showIntegrator() {
            if (ImGui::BeginTabItem("Integrator")) {
                if (auto r = selectImpl<core::Integrator>(graph->integrator, "Integrator", "Integrator##")) {
                    graph->integrator = r.value();

                }
                if (graph->integrator) {
                    InspectorPropertyVisitor visitor;
                    graph->integrator->accept(&visitor);
                }
                ImGui::EndTabItem();
            }
        }

        void showSettings() {
            if (ImGui::BeginTabItem("Settings")) {
                if (auto dim = GetInput("Film Dimension", graph->filmDimension)) {
                    graph->filmDimension = dim.value();
                }
                ImGui::EndTabItem();
            }
        }

        void showProperties() {
            if (ImGui::BeginTabItem("Properties")) {
                auto p = selected.lock();
                if (p) {
                    InspectorPropertyVisitor visitor;
                    p->accept(&visitor);
                }
                ImGui::EndTabItem();
            }
        }

        void showInspector() {

            if (ImGui::Begin("Inspector")) {
                if (ImGui::BeginTabBar("Tabs#00")) {
                    showProperties();
                    showSettings();
                    showCamera();
                    showSampler();
                    showIntegrator();

                    ImGui::EndTabBar();
                }
                ImGui::End();
            }
        }

        void showMenu() {
            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New")) {
                        graph.reset(new core::SceneGraph());
                    }
                    if (ImGui::MenuItem("Open")) {
                        auto filename = GetOpenFileNameWithDialog("Scene File\0*.json\0Any File\0*.*\0");
                        if (!filename.empty()) {
                            auto path = fs::path(filename);
                            std::ifstream in(filename);
                            serialize::InputArchive ar(in);
                            graph = std::dynamic_pointer_cast<core::SceneGraph>(serialize::ReadObject(ar));
                            sceneFilePath = fs::absolute(filename);
                        }
                    }
                    if (ImGui::MenuItem("Open (json)")) {
                        auto filename = GetOpenFileNameWithDialog("Json\0*.json\0Any File\0*.*\0");
                        if (!filename.empty()) {
                            auto path = fs::path(filename);
                            std::ifstream in(filename);

                            std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
                            json data = json::parse(str);
                            graph = std::make_shared<core::SceneGraph>();
                            graph->initialize(data);
                            sceneFilePath = fs::absolute(filename);
                        }
                    }
                    auto selectFilenameAndSave = [=]() {
                        auto filename = GetSaveFileNameWithDialog(nullptr);
                        if (!filename.empty()) {
                            std::ofstream out(filename);
                            serialize::OutputArchive ar(out);
                            serialize::WriteObject(ar, graph);
                        }
                        sceneFilePath = fs::absolute(filename);
                    };
                    auto errorCurrentSceneEmpty = [=]() {
                        showModal("Error", [=]() {
                            ImGui::Text("%s", "Current scene is empty!");
                            if (ImGui::Button("Close")) {
                                ImGui::CloseCurrentPopup();
                            }
                        });
                    };
                    auto errorNotSaved = [=]() {
                        showModal("Error", [=]() {
                            ImGui::Text("%s", "Current scene hasn't been saved!");
                            if (ImGui::Button("Close")) {
                                ImGui::CloseCurrentPopup();
                            }
                        });
                    };
                    if (ImGui::MenuItem("Save")) {
                        if (graph) {
                            if (sceneFilePath.empty()) {
                                selectFilenameAndSave();
                            } else {
                                std::ofstream out(sceneFilePath);
                                serialize::OutputArchive ar(out);
                                serialize::WriteObject(ar, graph);
                            }
                        } else {
                            errorCurrentSceneEmpty();
                        }
                    }

                    if (ImGui::MenuItem("Save As")) {
                        if (!graph) {
                            errorCurrentSceneEmpty();
                        } else {
                            selectFilenameAndSave();
                        }
                    }

                    if (ImGui::MenuItem("Import")) {
                        if (sceneFilePath.empty()) {
                            errorNotSaved();
                        } else {
                            auto filename = GetOpenFileNameWithDialog("Wavefront OBJ\0*.obj\0Any File\0*.*\0");
                            if (!filename.empty()) {
                                std::thread th([=]() {
                                    showModal("Importing", [=]() {

                                    });
                                    try {
                                        auto importer = std::dynamic_pointer_cast<core::MeshImporter>(
                                                CreateObject("WavefrontImporter"));
                                        auto result = importer->importMesh(filename);
                                        if (result.mesh) {
                                            graph->shapes.emplace_back(result.mesh);
                                            auto old = result.mesh->filename;
                                            auto relativePath = fs::relative(fs::path(old.append(".mesh")),
                                                                             fs::path(
                                                                                     sceneFilePath).parent_path()).string();
                                            CurrentPathGuard _guard;
                                            fs::current_path(fs::path(sceneFilePath).parent_path());
                                            result.mesh->writeToFile(relativePath);
                                        } else {
                                            log::log("Failed to import {}\n", filename);
                                        }
                                    } catch (std::exception &e) {
                                        log::log("Failed to import {} due to {}\n", filename, e.what());
                                    }
                                    closeModal();
                                });
                                th.detach();
                            }
                        }
                    }
                    if (ImGui::MenuItem("Close")) {
                        graph = nullptr;
                        sceneFilePath = fs::path();
                    }

                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }
        }

    public:
        MainWindow(int w, int h, const std::string &title) : AbstractMainWindow(w, h, title) {
            viewport = std::make_shared<Viewport>(ivec2(1280, 720));
            viewport->compileShader();
        }

        void update() override {
            modalFunc();
            SetupDockingSpace("DockingSpace");
            showMenu();
            if (graph) {
                showExplorer();
                showInspector();
                showView();
            }
            ImGui::ShowDemoWindow();
        }
    };

    std::shared_ptr<AbstractMainWindow> MakeMainWindow(int width, int height, const std::string &title) {
        return std::make_shared<MainWindow>(width, height, title);
    }
} // namespace miyuki::ui


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