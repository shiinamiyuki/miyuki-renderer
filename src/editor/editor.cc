//
// Created by Shiina Miyuki on 2019/3/19.
//

#include "editor.h"
#include <math/func.h>
#include <integrators/volpath/volpath.h>
//#include <integrators/mmlt/mmlt.h>
//#include <integrators/bdpt/bdpt.h>

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

#define _VOLPATH 0
#define _BDPT 1
#define _MMLT 2

Editor::Editor(int argc, char **argv) : GenericGUIWindow(argc, argv) {
    renderEngine.setGuiMode(true);
    renderEngine.processCommandLine(argc, argv);
    renderEngine.commitScene();
    pixelData.reserve(1920 * 1080 * 4);
    pixelDataBuffer.resize(1920 * 1080 * 4);
    renderEngine.updateFunc = [&]() {
        renderEngine.readPixelData(pixelDataBuffer, width, height);
        std::swap(pixelDataBuffer, pixelData);
    };
    Assert(renderEngine.integrator);
    if (typeid(*renderEngine.integrator) == typeid(VolPath)) {
        selectedIntegrator = _VOLPATH;
    }
//    else if (typeid(*renderEngine.integrator) == typeid(MultiplexedMLT)) {
//        selectedIntegrator = _MMLT;
//    } else if (typeid(*renderEngine.integrator) == typeid(BDPT)) {
//        selectedIntegrator = _BDPT;
//    }


    Log::SetLogLevel(Log::silent);
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
        ret |= ImGui::InputFloat("x ", &v->x(), 0.01f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
        ret |= ImGui::InputFloat("y ", &v->y(), 0.01f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
        ret |= ImGui::InputFloat("z ", &v->z(), 0.01f, 1.0f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::TreePop();
        return ret;
    }
    return false;
}

bool InputSpectrum(const char *prompt, Spectrum *color) {
    return ImGui::ColorEdit3(prompt, &color->r(), ImGuiColorEditFlags_Float);
}

bool InputFloat(const char *prompt, Float *v) {
    return ImGui::InputFloat(prompt, v, 0, 0, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue);
}

bool InputBool(const char *prompt, bool *b) {
    return ImGui::Checkbox(prompt, b);
}

// Not thread safe!
bool InputString(const char *prompt, std::string &s) {
    static char text[1024];
    for (int i = 0; i < s.length(); i++) {
        text[i] = s[i];
    }
    text[s.length()] = 0;
    if (ImGui::InputText(prompt, text, 1024, ImGuiInputTextFlags_EnterReturnsTrue)) {
        s = text;
        return true;
    }
    return false;
}

static const char *integratorName[] = {
        "Volumetric Path Tracer",
        "Guided Path Tracer",
//        "Bidirectional Path Tracer",
//        "Multiplexed Metropolis Light Transport"
};


void Editor::integratorWindow() {
    ImGui::SetNextWindowPos(ImVec2(250, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Integrator Menu", nullptr, 0)) {
        return;
    }
    bool modified = false;
    if (ImGui::TreeNode("Integrator")) {
        for (int n = 0; n < 2; n++) {
            if (ImGui::Selectable(integratorName[n], selectedIntegrator == n)) {
                selectedIntegrator = n;
                modified = true;
            }
        }

        if (ImGui::TreeNode("Settings")) {
            Float minDepth, maxDepth;
            Float spp;
            minDepth = IO::deserialize<Float>(renderEngine.description["integrator"]["minDepth"]);
            maxDepth = IO::deserialize<Float>(renderEngine.description["integrator"]["maxDepth"]);
            spp = IO::deserialize<Float>(renderEngine.description["integrator"]["spp"]);

            if (InputFloat("spp", &spp)) {
                renderEngine.description["integrator"]["spp"] = IO::serialize(spp);
                modified = true;
            }
            if (InputFloat("minDepth", &minDepth)) {
                renderEngine.description["integrator"]["minDepth"] = IO::serialize(minDepth);
                modified = true;
            }
            if (InputFloat("maxDepth", &maxDepth)) {
                renderEngine.description["integrator"]["maxDepth"] = IO::serialize(maxDepth);
                modified = true;
            }
            
            if (selectedIntegrator == _MMLT) {
                Float nChains, nDirect;
                nChains = IO::deserialize<Float>(renderEngine.description["integrator"]["nChains"]);
                nDirect = IO::deserialize<Float>(renderEngine.description["integrator"]["nDirect"]);
                if (InputFloat("nChains", &nChains)) {
                    renderEngine.description["integrator"]["nChains"] = IO::serialize(nChains);
                    modified = true;
                }
                if (InputFloat("nDirect", &nDirect)) {
                    renderEngine.description["integrator"]["nDirect"] = IO::serialize(nDirect);
                    modified = true;
                }
            } else {
                bool progressive = renderEngine.description["integrator"].get("progressive").getBool();
                if (InputBool("progressive", &progressive)) {
                    renderEngine.description["integrator"]["progressive"] = progressive;
                    modified = true;
                }
            }

            ImGui::TreePop();
        }
        if (modified) {
            try {
                if (selectedIntegrator == _MMLT) {
                    renderEngine.description["integrator"]["type"] = std::string("mlt");
                } else if (selectedIntegrator == _VOLPATH) {
                    renderEngine.description["integrator"]["type"] = std::string("volpath");
                } else if (selectedIntegrator == _BDPT) {
                    renderEngine.description["integrator"]["type"] = std::string("bdpt");
                }
                renderEngine.loadIntegrator();
            } catch (std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("View")) {
        if (ImGui::Checkbox("Rendered", &runIntegrator)) {
            if (runIntegrator) {
                startRenderThread();
            } else {
                stopRenderThread();
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Render Setting")) {
        ImGui::TreePop();
    }
}

bool InputTransform(const char *prompt, Transform *transform) {
    Vec3f translation = transform->translation,
            rotation = transform->rotation;
    Float scale = transform->scale;
    rotation = RadiansToDegrees(rotation);
    if (InputVec3f("Translation", &translation)
        || InputVec3f("Rotation", &rotation)
        || InputFloat("Scale", &scale)) {
        if (scale <= 0) {
            Log::log(Log::error, "scale must be greater than 0\n");
            return false;
        }
        rotation = DegreesToRadians(rotation);
        *transform = Transform(translation, rotation, scale);
        return true;
    }
    return false;
}

void Editor::treeNodeCameras() {
    if (ImGui::TreeNode("Camera")) {

        Vec3f translation = IO::deserialize<Vec3f>(renderEngine.description["camera"]["translation"]),
                rotation = IO::deserialize<Vec3f>(renderEngine.description["camera"]["rotation"]);
        Float lensRadius = IO::deserialize<Float>(renderEngine.description["camera"]["lensRadius"]);
        Float focalDistance = IO::deserialize<Float>(renderEngine.description["camera"]["focalDistance"]);
        if (InputVec3f("Translation", &translation)
            || InputVec3f("Rotation", &rotation)
            || InputFloat("Lens Radius", &lensRadius)
            || InputFloat("Focal Distance", &focalDistance)) {
            rerender = true;
            renderEngine.description["camera"]["translation"] = IO::serialize(translation);
            renderEngine.description["camera"]["rotation"] = IO::serialize(rotation);
            renderEngine.description["camera"]["lensRadius"] = IO::serialize(lensRadius);
            renderEngine.description["camera"]["focalDistance"] = IO::serialize(focalDistance);
            renderEngine.loadCamera();
        }

        ImGui::TreePop();
    }
}

void Editor::treeNodeShapes() {
    if (ImGui::TreeNode("Shapes")) {
        static char search[1024];
        if (ImGui::InputText("search: ", search, 1024)) {
            for (auto &i: shapeSearch)
                i.second.match(search);
        }
        bool modified = false;
        for (const auto &mesh:renderEngine.description["shapes"].getObject()) {
            if (ImGui::TreeNode(mesh.first.c_str())) {
                for (auto &i: shapeSearch[mesh.first].matched) {
                    if (ImGui::TreeNode(i.c_str())) {
                        auto &s = renderEngine.description["shapes"][mesh.first][i].getString();
                        static char input[1024];
                        for (int k = 0; k < s.length(); k++) {
                            input[k] = s[k];
                        }
                        input[s.length()] = 0;

                        if (ImGui::InputText("material", input, 1024, ImGuiInputTextFlags_EnterReturnsTrue)) {
                            if (renderEngine.description["materials"].hasKey(input)) {
                                modified = true;
                                renderEngine.description["shapes"][mesh.first][i] = Json::JsonObject(
                                        std::string(input));
                            } else {
                                Log::log(Log::error, "Does not have material named {}\n", input);
                            }
                        }
                        if (modified) {
                            renderEngine.updateMaterials();
                            updateShape();
                            rerender = true;
                        }
                        ImGui::TreePop();
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void Editor::treeNodeMaterials() {
    if (ImGui::TreeNode("Materials")) {
        static char search[1024];
        if (ImGui::InputText("search: ", search, 1024)) {
            materialSearch.match(search);
        }

        for (const auto &i:materialSearch.matched) {
            auto factory = renderEngine.scene.getMaterialFactory();
            if (ImGui::TreeNode(i.c_str())) {
                bool modified = false;
                auto material = factory->getMaterialByName(i);
                Assert(material);
                auto json = material->toJson();
                Spectrum ka, kd, ks;
                ka = IO::deserialize<Spectrum>(json["ka"]["albedo"]);
                kd = IO::deserialize<Spectrum>(json["kd"]["albedo"]);
                ks = IO::deserialize<Spectrum>(json["ks"]["albedo"]);
                Float Tr, Ni, roughness, emission;
                Tr = IO::deserialize<Float>(json["Tr"]);
                Ni = IO::deserialize<Float>(json["Ni"]);
                roughness = IO::deserialize<Float>(json["roughness"]);
                emission = IO::deserialize<Float>(json["emission"]);
                if (InputSpectrum("ka", &ka)) {
                    modified = true;
                }
                if (InputSpectrum("kd", &kd)) {
                    modified = true;
                }
                if (InputSpectrum("ks", &ks)) {
                    modified = true;
                }
                if (InputFloat("Tr", &Tr)) {
                    modified = true;
                }
                if (InputFloat("Ni", &Ni)) {
                    modified = true;
                }
                if (InputFloat("roughness", &roughness)) {
                    modified = true;
                }
                if (InputFloat("emission", &emission)) {
                    modified = true;
                }
                if (ImGui::TreeNode("Textures")) {
                    if (InputString("ka.texture", json["ka"]["texture"].getString())) {
                        modified = true;
                    }
                    if (InputString("kd.texture", json["kd"]["texture"].getString())) {
                        modified = true;
                    }
                    if (InputString("ks.texture", json["ks"]["texture"].getString())) {
                        modified = true;
                    }
                    ImGui::TreePop();
                }
                if (modified) {
                    json["ka"]["albedo"] = IO::serialize(ka);
                    json["kd"]["albedo"] = IO::serialize(kd);
                    json["ks"]["albedo"] = IO::serialize(ks);
                    json["Tr"] = IO::serialize(Tr);
                    json["Ni"] = IO::serialize(Ni);
                    json["roughness"] = IO::serialize(roughness);
                    json["emission"] = IO::serialize(emission);
                    factory->modifyMaterialByNameFromJson(i, json);
                    renderEngine.description["materials"][i] = json;
                    renderEngine.updateMaterials();
                    rerender = true;
                }

                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void Editor::treeNodeObject() {
    if (ImGui::TreeNode("Selected Object")) {
        if (pickedObject.valid()) {
            ImGui::Text("%s", fmt::format("Geometry id:{}, Primitive id:{}",
                                          pickedObject.geomId, pickedObject.primId).c_str());
            ImGui::Text("%s", fmt::format("Mesh name: {}",
                                          pickedObject.primitive->instance->name).c_str());
            ImGui::Text("%s", fmt::format("Object name: {}",
                                          pickedObject.primitive->name()).c_str());
            ImGui::Text("%s", fmt::format("Material name: {}",
                                          renderEngine.description["shapes"]
                                          [pickedObject.primitive->instance->name]
                                          [pickedObject.primitive->name()].getString()).c_str());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Objects")) {
        for (auto &object : renderEngine.description["objects"].getArray()) {
            if (ImGui::TreeNode(object["name"].getString().c_str())) {
                auto transform = IO::deserialize<Transform>(object["transform"]);
                if (InputTransform("Transform", &transform)) {
                    object["transform"] = IO::serialize(transform);
                    for (auto &i: renderEngine.scene.allInstances()) {
                        if (i->name == object["name"].getString()) {
                            i->resetTransform(transform);
                            rerender = true;
                        }
                    }
                }
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}


void Editor::treeNodeFile() {
    if (ImGui::TreeNode("File")) {
        if (ImGui::SmallButton("Save")) {
            std::ofstream out(renderEngine.sceneFileName);
            if (!out) {
                Log::log(Log::error, "Cannot save {}\n", renderEngine.sceneFileName);
            } else {
                out << renderEngine.description.toString();
                fmt::print("Saved {}\n", renderEngine.sceneFileName);
            }
        }
        ImGui::TreePop();
    }

}

void Editor::treeNodeScene() {
    if (ImGui::TreeNode("Scene")) {
        Spectrum ambientLight = IO::deserialize<Spectrum>(renderEngine.description["integrator"]["ambientLight"]);
        bool modified = false;
        if (InputSpectrum("Ambient Light", &ambientLight)) {
            renderEngine.description["integrator"]["ambientLight"] = IO::serialize<Spectrum>(ambientLight);
            modified = true;
        }
        if (InputString("Environment Mapping", renderEngine.description["integrator"]["envMap"].getString())) {
            modified = true;
        }
        if (modified) {
            renderEngine.loadIntegrator();
            rerender = true;
        }
        ImGui::TreePop();
    }
}

void Editor::mainEditorWindow() {
    ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Editor Menu", nullptr, 0)) {
        treeNodeScene();
        treeNodeCameras();
        treeNodeObject();
        treeNodeShapes();
        treeNodeMaterials();
        treeNodeFile();
        showDebug();
        objectPicker();
    }

    integratorWindow();
}

void Editor::objectPicker() {
    static MemoryArena arena;
    if (!ImGui::IsMouseHoveringAnyWindow()) {
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
    updateMaterial();
    updateShape();
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
    if (runIntegrator) {
        stopRenderThread();
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

void Editor::updateMaterial() {
    materialSearch.all.clear();
    for (const auto &i: renderEngine.description["materials"].getObject()) {
        materialSearch.all.emplace_back(i.first);
    }
    materialSearch.match("");

}

void Editor::updateShape() {
    for (const auto &mesh : renderEngine.description["shapes"].getObject()) {
        auto &name = mesh.first;
        shapeSearch[name] = StringSearch();
        for (const auto &i: mesh.second.getObject()) {
            shapeSearch[name].all.emplace_back(i.first);
        }
        shapeSearch[name].match("");
    }
}

void Editor::startRenderThread() {
    if (!renderEngine.integrator) {
        renderEngine.loadIntegrator();
        renderEngine.scene.getFilm()->clear();
    }
    renderThread = std::make_unique<std::thread>([&]() {
        renderEngine.commitScene();
        renderEngine.startRender();
        renderEngine.imageSize(width, height);
        renderEngine.exec();
    });
}

void Editor::stopRenderThread() {
    renderEngine.stopRender();
    renderThread->join();
    renderEngine.integrator = nullptr;
}


static inline bool containsCaseInsensitive(const std::string &s, const std::string &p) {
    auto it = std::search(
            p.begin(), p.end(),
            s.begin(), s.end(),
            [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); }
    );
    return it != p.end();
}

std::vector<std::string> &StringSearch::match(const std::string &s) {
    matched.clear();
    for (const auto &i: all) {
        if (s.empty() || containsCaseInsensitive(s, i)) {
            matched.emplace_back(i);
        }
    }
    selected.clear();
    selected.resize(matched.size());
    return matched;
}
