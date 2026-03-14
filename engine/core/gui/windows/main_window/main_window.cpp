#include "main_window.h"
#include"glad/glad.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/test_render_engine.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/camera/camera_handeling/camera_handeling.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/camera/camera_input/camera_input.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/View_port_panel/FBO_taker/test_render_engine_FBO.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/scene/scene_class/scene.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/hierarchy_panel/hierarchy_panel.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/acpects/texture_loader/texture_loader.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/menu_bar/menu_bar.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\GUI\Panels\project_manager\project_manager.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/default_theme.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/command_panel/command_panel.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/inspector_panel/inspector_panel.h"
#include"E:\PROJECTS\glfw\learning\project_FLAGE\engine\Editor\GUI\Panels\content_browser\content_browser.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/Editor/GUI/Panels/dockSpaceUI.h"
#include "E:/PROJECTS/glfw/learning/project_FLAGE/Libraries/include/STB/stb_image.h"
#include <E:/PROJECTS/glfw/learning/project_FLAGE/engine_core.h>
#include <GLFW/glfw3.h>
#include <json.hpp>
#include <iostream>

#define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/inputs/core_inputs/base_input_handler.h"

// Simple GLFW error callback to surface errors to console
static void GLFWErrorCallback(int error, const char* description) {
    std::cerr << "GLFW ERROR (" << error << "): " << (description ? description : "null") << std::endl;
}

void class_main_window::window_init() {
    std::cout << "hello\n";

    // Install error callback BEFORE glfwInit so we catch init-time errors.
    glfwSetErrorCallback(GLFWErrorCallback);

    if (!glfwInit()) {
        std::cerr << "ERROR_Failed_to_init_GLFW_\n";
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* main_window = glfwCreateWindow(1280, 720, "flag_engine", NULL, NULL);
    if (!main_window) {
        std::cerr << "ERROR_Failed_to_create_main_window_\n";
        glfwTerminate();
        return;
    }
    GLFWimage image[1];
	image[0].pixels = stbi_load("resources/icons/flage_icon_main.png", &image[0].width, &image[0].height, 0, 4);
    if (image[0].pixels) {
        glfwSetWindowIcon(main_window,1,image);
    }
    else {
        std::cerr << "failed to create icon\n";
    }
    // Make context current
    glfwMakeContextCurrent(main_window);

    // Load GLAD once, right after context is current
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR_Failed_to_initialize_GLAD_\n";
        glfwDestroyWindow(main_window);
        glfwTerminate();
        return;
    }

    // Diagnostic: ensure key GL entrypoints are loaded before creating shaders
    // glad exposes the raw function pointer variables (glad_glCreateShader etc.) in glad.c
    extern PFNGLCREATESHADERPROC glad_glCreateShader; // forward-declare the glad pointer
    if (glad_glCreateShader == NULL) {
        std::cerr << "FATAL: OpenGL function pointers not loaded (glad_glCreateShader == NULL)." << std::endl;
        std::cerr << "Possible causes: GL context not current, wrong loader, or glad wasn't initialized correctly." << std::endl;
        std::cerr << "GL version string (if any): ";
        const GLubyte* ver = nullptr;
        // Try to read version (may be null if loader really failed)
        ver = glGetString ? glGetString(GL_VERSION) : nullptr;
        if (ver) std::cerr << ver << std::endl; else std::cerr << "unknown\n";
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        glfwDestroyWindow(main_window);
        glfwTerminate();
        return;
    }

    // add right after gladLoadGLLoader(...)
    int gladLoaded = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    std::cerr << "gladLoadGLLoader returned: " << gladLoaded << std::endl;
    const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    std::cerr << "glGetString(GL_VERSION) = " << (ver ? ver : "null") << std::endl;
    // check a couple of critical pointers from glad.c
    extern PFNGLCREATESHADERPROC glad_glCreateShader;
    extern PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
    std::cerr << "glad_glCreateShader = " << (void*)glad_glCreateShader << std::endl;
    std::cerr << "glad_glCreateProgram = " << (void*)glad_glCreateProgram << std::endl;

    static InspectorPanel inspector;
    static HierarchyPanel hierarchy;
    static ContentBrowser content_browser;
    // Initial viewport setup
    int w = 0, h = 0;
    glfwGetFramebufferSize(main_window, &w, &h);
    glViewport(0, 0, w, h);

    glfwSetFramebufferSizeCallback(main_window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        });

    // Renderer
    test_render_engine renderer;
    renderer.ready_render();

    // Global GL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Camera + input
    Camera camera;
    CameraInput camInput(&camera);
    FBO viewportFBO(1280, 720);
    camInput.AttachCallbacks(main_window);

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    CustomImGuiTheme::Apply3dsMaxTheme();
    ImGui_ImplGlfw_InitForOpenGL(main_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Scene setup
    Scene scene;
    CreatePanel create_panel(&scene);
    MenuBar menu_bar(&scene);
    hierarchy.scene = &scene;
    inspector.scene = &scene;

    // Load texture
    GLuint grid_texture = loadTexture("E:/sky_10_2k.png");

    // Plane entity
    Entity plane = scene.AddEntity("plane", ShapeType::Sphere);
    scene.world.getTransform(plane).position = { 0.0f, -3.0f, 0.0f };
    scene.world.getTransform(plane).scale = { 40.0f, 40.0f, 40.0f };
    scene.world.getTransform(plane).rotation = { 180.0f, 0.0f, 0.0f };
    // Around line 126, change the plane rendering setup:
    auto& cubeRender3 = scene.world.getRender(plane);
    cubeRender3.ambientStrength = 0.3f;    // <-- changed from 0.2f
    cubeRender3.specularStrength = 0.5f;
    cubeRender3.shininess = 32.0f;         // <-- changed from 1.0f
    cubeRender3.useTexture = true;
    cubeRender3.textureID = grid_texture;
    cubeRender3.visible = true;
    scene.AddLightEntity("Sun",
        LightType::Directional,
        glm::vec3(0.0f),
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(1.0f, 1.0f, 0.9f),
        1.0f);

    // ADD A TEST LIGHT:
    Entity dirLight = scene.AddLightEntity(
        "MainLight",
        LightType::Directional,
        glm::vec3(0.0f),           // position (unused for directional)
        glm::vec3(0.0f, -1.0f, 0.0f), // direction (pointing down)
        glm::vec3(1.0f, 1.0f, 1.0f),  // white color
        1.0f                           // intensity
    );
     // <-- CHANGE FROM 1.0 to 32.0
    DockspaceUI ui;

    // Shadow renderer init AFTER context + GLAD
   test_render_engine::shadow.InitShaders(4096, 4096, 2048);

    while (!glfwWindowShouldClose(main_window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ProjectManager::RenderProjectLauncher();
        // UI panels
        ui.Render();
        create_panel.Render();
        hierarchy.Render();
        inspector.Render();
        menu_bar.Render();
        content_browser.OnImGuiRender();
        // Camera input
        if (viewportFBO.IsHovered()) {
            camInput.SetAllowInput(true);
            camera.UpdateSpeed(main_window);
            camInput.Update(main_window, viewportFBO.width, viewportFBO.height);
        }
        else {
            camInput.allowInput = false;
        }

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render viewport (FBO handles shadow + scene internally)
        viewportFBO.RenderPanel(camera, renderer, scene);

        // End ImGui frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(main_window);

        if (glfwGetKey(main_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(main_window, true);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    // If viewports were enabled, destroy platform windows first
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::DestroyPlatformWindows();
    }
    ImGui::DestroyContext();

    glfwDestroyWindow(main_window);
    glfwTerminate();
}

void class_main_window::window_terminate() {
    main_window_running = false;
    std::cerr << "SUCESSFULLY:_closed_main_window_\n";
    glfwTerminate();
}
