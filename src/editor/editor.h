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
    struct StringSearch {
        std::vector<std::string> all;
        std::vector<std::string> matched;
        std::vector<bool> selected;

        std::vector<std::string> &match(const std::string &s);

    };

    class Editor : public GenericGUIWindow {
        RenderEngine renderEngine;
        int selectedIntegrator;
        int width = 1980, height = 1080;
        std::vector<uint8_t> pixelData, pixelDataBuffer;
        GLFWwindow *window = nullptr;
        Float scale = 1.0f;
        std::unique_ptr<std::thread> renderThread;

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
        StringSearch materialSearch;
        std::unordered_map<std::string, StringSearch> shapeSearch;

        bool runIntegrator = false;

        void treeNodeShapes();

        void treeNodeCameras();

        void treeNodeMaterials();

        void treeNodeObject();

        void treeNodeFile();

        void startRenderThread();

        void stopRenderThread();

    public:
        Editor(int argc, char **argv);

        bool rerender = false;

        void mainEditorWindow();

        void integratorWindow();

        void objectPicker();

        void handleEvents() {
            rerender = false;
            mainEditorWindow();
            if (rerender) {
                update();
            }
        }

        void showDebug();

        void render() override {
            glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());
//              ImGui::ShowDemoWindow(nullptr);
        }

        void updateMaterial();

        void updateShape();

        void update() override {
            if (!runIntegrator) {
                renderEngine.commitScene();
                renderEngine.renderPreview(pixelData, width, height);
            }
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
