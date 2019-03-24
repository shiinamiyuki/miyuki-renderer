//
// Created by Shiina Miyuki on 2019/3/24.
//

#ifndef MIYUKI_EDITOR_H
#define MIYUKI_EDITOR_H

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


namespace Miyuki {
    class Editor : public GenericGUIWindow {
        RenderEngine renderEngine;
        int width = 1980, height = 1080;
        std::vector<uint8_t> pixelData;
        GLFWwindow *window = nullptr;
        Float scale = 1.0f;

        struct PickedObject {
            int geomId = -1, primId = -1;
            const Primitive *primitive = nullptr;

            void reset() {
                primitive = nullptr;
                geomId = primId = -1;
            }

            bool valid() const {
                return primitive && geomId != -1 && primId != -1;
            }
        };

        PickedObject pickedObject;
    public:
        Editor(int argc, char **argv);

        bool rerender = false;

        void mainEditorWindow();

        void objectPicker();

        void handleEvents() {
            rerender = false;
            mainEditorWindow();
            if (rerender) {
                renderEngine.updateCameraInfoToParameterSet();
                update();
            }

        }

        void showDebug();

        void render() override {
            glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());
        }

        void update() override {
            renderEngine.commitScene();
            renderEngine.renderPreview(pixelData, width, height);
        }

        void show() override;

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

#endif //MIYUKI_EDITOR_H
