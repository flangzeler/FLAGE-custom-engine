#include <windows.h>
#include <iostream>
#include<E:/PROJECTS/glfw/learning/project_FLAGE/engine/core/renderer/render_engines/test_engine/test_render_engine.h>
#include"main_window.h"
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include "imgui.h" 
#include "imgui_impl_glfw.h" 
#include "imgui_impl_opengl3.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

static void EnsureConsoleAttached() {
#ifdef _WIN32
    if (!::IsDebuggerPresent()) {
        if (::AllocConsole()) {
            FILE* fp;
            freopen_s(&fp, "CONOUT$", "w", stdout);
            freopen_s(&fp, "CONOUT$", "w", stderr);
            freopen_s(&fp, "CONIN$", "r", stdin);
            std::ios::sync_with_stdio(true);
        }
    }
#endif
}

int main()
{
    EnsureConsoleAttached();

    class_main_window window;

    window.window_init();

    return 0;
}



