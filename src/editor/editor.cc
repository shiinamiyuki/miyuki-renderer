//
// Created by Shiina Miyuki on 2019/3/19.
//

#include "editor.h"
#include <math/func.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}


int main(int argc, char **argv) {
    using namespace Miyuki;
    try {
        std::unique_ptr<Editor> editor(new Editor(argc, argv));
        editor->show();
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}

Editor::Editor(int argc, char **argv) : GenericGUIWindow(argc, argv) {
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

bool InputVec3f(const char *prompt, Vec3f *v) {
    if (ImGui::TreeNode(prompt)) {
        bool ret = false;
        ret |= ImGui::InputFloat("x ", &v->x(), 0.01f, 1.0f, "%.3f");
        ret |= ImGui::InputFloat("y ", &v->y(), 0.01f, 1.0f, "%.3f");
        ret |= ImGui::InputFloat("z ", &v->z(), 0.01f, 1.0f, "%.3f");
        ImGui::TreePop();
        return ret;
    }
    return false;
}

void Editor::mainEditorWindow() {

    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Editor Menu", nullptr, 0)) {
        ImGui::End();
        return;
    }
    if (ImGui::TreeNode("Camera")) {

        Vec3f translation = renderEngine.getMainCamera()->translation(),
                rotation = renderEngine.getMainCamera()->rotation();
        rotation = RadiansToDegrees(rotation);
        if (InputVec3f("Translation", &translation) || InputVec3f("Rotation", &rotation)) {
            rerender = true;
            rotation = DegressToRadians(rotation);
            renderEngine.getMainCamera()->moveTo(translation);
            renderEngine.getMainCamera()->rotateTo(rotation);
            renderEngine.updateCameraInfoToParameterSet();
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Object")) {
        if (pickedObject.valid()) {
            ImGui::Text(fmt::format("Geometry id:{}, Primitive id:{}",
                                    pickedObject.geomId, pickedObject.primId).c_str());
            ImGui::Text(fmt::format("Object name: {}",
                                    pickedObject.primitive->name()).c_str());
            ImGui::Text(fmt::format("Material name: {}",
                                    renderEngine.description["shapes"][pickedObject.primitive->name()].getString()).c_str());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Shapes")) {

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Materials")) {

        ImGui::TreePop();
    }
    showDebug();
    objectPicker();

}

void Editor::objectPicker() {
    static MemoryArena arena;
    if (!ImGui::IsMouseHoveringWindow()) {


        auto &io = ImGui::GetIO();
        if (io.MouseClicked[0]) {
            Seed seed(rand());
            RandomSampler sampler(&seed);
            Point2i clickedPos(io.MouseClickedPos->x, io.MouseClickedPos->y);
            int w, h;
            glfwGetWindowSize(window, &w, &h);
            clickedPos.y() -= h - height;
            if (clickedPos.x() >= 0 && clickedPos.x() < renderEngine.scene.filmDimension().x() &&
                clickedPos.y() >= 0 && clickedPos.y() < renderEngine.scene.filmDimension().y()) {
                auto ctx = renderEngine.getScene()->getRenderContext(clickedPos, &arena, &sampler);
                Intersection intersection;
                if (renderEngine.getScene()->intersect(ctx.primary, &intersection)) {
                    pickedObject = PickedObject{intersection.geomId, intersection.primId, intersection.primitive};
                }
            } else {
                pickedObject.reset();
            }
        }
    }
}

void Editor::show() {
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

        handleEvents();
        render();
        // Rendering
        ImGui::Render();


        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }

}

void Editor::showDebug() {
    if (ImGui::TreeNode("Debug")) {
        auto &io = ImGui::GetIO();
        if (ImGui::IsMousePosValid())
            ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
        else
            ImGui::Text("Mouse pos: <INVALID>");
        ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
        ImGui::Text("Mouse down:");
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
            if (io.MouseDownDuration[i] >= 0.0f) {
                ImGui::SameLine();
                ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]);
            }
        ImGui::Text("Mouse clicked:");
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
            if (ImGui::IsMouseClicked(i)) {
                ImGui::SameLine();
                ImGui::Text("b%d", i);
            }
        ImGui::Text("Mouse dbl-clicked:");
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
            if (ImGui::IsMouseDoubleClicked(i)) {
                ImGui::SameLine();
                ImGui::Text("b%d", i);
            }
        ImGui::Text("Mouse released:");
        for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
            if (ImGui::IsMouseReleased(i)) {
                ImGui::SameLine();
                ImGui::Text("b%d", i);
            }
        ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

        ImGui::Text("Keys down:");
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
            if (io.KeysDownDuration[i] >= 0.0f) {
                ImGui::SameLine();
                ImGui::Text("%d (%.02f secs)", i, io.KeysDownDuration[i]);
            }
        ImGui::Text("Keys pressed:");
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
            if (ImGui::IsKeyPressed(i)) {
                ImGui::SameLine();
                ImGui::Text("%d", i);
            }
        ImGui::Text("Keys release:");
        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
            if (ImGui::IsKeyReleased(i)) {
                ImGui::SameLine();
                ImGui::Text("%d", i);
            }
        ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "",
                    io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");

        ImGui::Text("NavInputs down:");
        for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++)
            if (io.NavInputs[i] > 0.0f) {
                ImGui::SameLine();
                ImGui::Text("[%d] %.2f", i, io.NavInputs[i]);
            }
        ImGui::Text("NavInputs pressed:");
        for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++)
            if (io.NavInputsDownDuration[i] == 0.0f) {
                ImGui::SameLine();
                ImGui::Text("[%d]", i);
            }
        ImGui::Text("NavInputs duration:");
        for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++)
            if (io.NavInputsDownDuration[i] >= 0.0f) {
                ImGui::SameLine();
                ImGui::Text("[%d] %.2f", i, io.NavInputsDownDuration[i]);
            }

        ImGui::Button("Hovering me sets the\nkeyboard capture flag");
        if (ImGui::IsItemHovered())
            ImGui::CaptureKeyboardFromApp(true);
        ImGui::SameLine();
        ImGui::Button("Holding me clears the\nthe keyboard capture flag");
        if (ImGui::IsItemActive())
            ImGui::CaptureKeyboardFromApp(false);
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        ImGui::Text("Window Size: %f %f", ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
        ImGui::Text("GLFW Window Size: %d %d", w, h);
        int p[2];
        glGetIntegerv(GL_CURRENT_RASTER_POSITION, p);
        ImGui::Text("GL_CURRENT_RASTER_POSITION: %d %d", p[0], p[1]);
        ImGui::TreePop();
    }

}

