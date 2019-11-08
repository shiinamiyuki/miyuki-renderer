#include <api/ui/ui.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <api/defs.h>
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

    class MainWindow::Impl {

      public:
        MainWindow *mw;
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
                for (auto &i : mw->children) {
                    i->draw();
                }
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
    MainWindow::MainWindow(int width, int height, const std::string &title) {
        impl = new Impl(width, height, title);
        impl->mw = this;
    }
    MainWindow::~MainWindow() { delete impl; }
    void MainWindow::draw() { impl->draw(); }

    void Text::draw() { ImGui::Text("%s", text.c_str()); }

    void TreeNode::draw() {
        ImGui::PushID((void *)this);
        if (ImGui::TreeNode(name.c_str())) {
            for (auto &i : children) {
                i->draw();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
    void Window::draw() {
        if (ImGui::Begin(name.c_str())) {
            for (auto &i : children) {
                i->draw();
            }
            ImGui::End();
        }
    }
    void DemoWindow::draw() { ImGui::ShowDemoWindow(&showed); }

    void DockingSpace::draw() {
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

        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

        for (auto &i : children) {
            i->draw();
        }
        ImGui::End();
    }

    void Button::draw() {
        if (ImGui::Button(text.c_str())) {
            callback();
        }
    }

    void CheckBox::draw() {
        if (ImGui::Checkbox(text.c_str(), &checked)) {
            callback(checked);
        }
    }

    void Slider::draw() {
        if (ImGui::SliderFloat(label.c_str(), &value, min, max)) {
            callback(value);
        }
    }

    std::optional<int> GetInput(const char *label, int value) {
        int tmp = value;
        if (ImGui::InputInt(label, &tmp, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
            return tmp;
        }
        return {};
    }
    std::optional<float> GetInput(const char *label, float value) {
        float tmp = value;
        if (ImGui::InputFloat(label, &tmp, 0.1, 1.0f, "0.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            return tmp;
        }
        return {};
    }
    std::optional<Vec3f> GetInput(const char *label, const Vec3f &value) {
        Vec3f tmp = value;
        if (ImGui::InputFloat3(label, (float *)&tmp, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue)) {
            return tmp;
        }
        return {};
    }
    std::optional<std::string> GetInput(const char *label, const std::string &s) {
        std::vector<char> buffer(2014);
        for (int i = 0; i < s.length(); i++)
            buffer[i] = s[i];
        if (ImGui::InputText(label, &buffer[0], buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue)) {
            return std::string(&buffer[0]);
        }
        return {};
    }


} // namespace miyuki::ui