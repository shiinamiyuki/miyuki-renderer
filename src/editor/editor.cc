//
// Created by Shiina Miyuki on 2019/3/19.
//

#include "miyuki.h"
#include "thirdparty/imgui/imgui.h"
#include "thirdparty/imgui/imgui_impl_glfw.h"
#include "thirdparty/imgui/imgui_impl_opengl2.h"

#include <stdio.h>

using namespace Miyuki;

#include <stdio.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <iostream>
#include <GLFW/glfw3.h>
#include <renderer/renderengine.h>
#include "gui.h"


// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace Miyuki {
    class Editor : public GenericGUIWindow {
        RenderEngine renderEngine;
        int width = 1280, height = 720;
        std::vector<uint8_t> pixelData;
        GLFWwindow *window = nullptr;
    public:
        Editor(int argc, char **argv) : GenericGUIWindow(argc, argv) {
            renderEngine.setGuiMode(true);
            renderEngine.processCommandLine(argc, argv);
            renderEngine.commitScene();
            pixelData.reserve(1920 * 1080 * 4);
            glfwSetErrorCallback(glfw_error_callback);
            if (!glfwInit())
                std::exit(1);
            window = glfwCreateWindow(width, height, "Miyuki Renderer Editor", nullptr, nullptr);
            if (window == nullptr)
                std::exit(1);
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();
            (void) io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
            //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsClassic();

            // Setup Platform/Renderer bindings
            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL2_Init();


            renderEngine.imageSize(width, height);
            glfwSetWindowSize(window, width, height);
            
        }

        void render() override {
            glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());
            static bool b = true;
            ImGui::ShowDemoWindow(&b);
            
        }

        void update() override {
            renderEngine.renderPreview(pixelData, width, height);
        }

        void show() override {
            update();
            ImVec4 clear_color = ImVec4(0, 0, 0, 1.00f);
            // Main loop
            while (!glfwWindowShouldClose(window)) {

                int display_w, display_h;
                glfwGetFramebufferSize(window, &display_w, &display_h);
                glViewport(0, 0, display_w, display_h);
                glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
                glClear(GL_COLOR_BUFFER_BIT);
                glfwPollEvents();

                // Start the Dear ImGui frame
                ImGui_ImplOpenGL2_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();


                render();
                // Rendering
                ImGui::Render();


                ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

                glfwMakeContextCurrent(window);
                glfwSwapBuffers(window);
                //std::this_thread::sleep_for(std::chrono::milliseconds(60));
            }
        }

        ~Editor() override {
            ImGui_ImplOpenGL2_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();

            glfwDestroyWindow(window);
            glfwTerminate();
            fmt::print("Exit\n");
        }
    };
}

int main(int argc, char **argv) {
    using namespace Miyuki;
    try {
        Editor editor(argc, argv);
        editor.show();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}