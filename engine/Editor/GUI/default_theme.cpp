#include"default_theme.h"

void CustomImGuiTheme::Apply3dsMaxTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    ImGui::StyleColorsDark(); // base dark style

    // Window title bars (keep neutral gray)
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);

    // Window background
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);

    // Section headers (neutral only)
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.26f, 0.26f, 1.0f);

    // Buttons (neutral base, blue highlight on hover/active)
    colors[ImGuiCol_Button] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.45f, 0.85f, 1.0f); // blue hover
    colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.35f, 0.70f, 1.0f); // deeper blue active

    // Frames (inputs, sliders, checkboxes)
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.45f, 0.85f, 1.0f); // blue hover
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.35f, 0.70f, 1.0f); // blue active

    // Checkboxes (neutral box, blue tick)
    colors[ImGuiCol_CheckMark] = ImVec4(0.20f, 0.45f, 0.85f, 1.0f);

    // Tabs (neutral base, blue highlight when selected)
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.45f, 0.75f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 0.7f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.11f, 0.11f, 0.7f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);

    // Style tweaks
    style.FrameRounding = 3.0f;
    style.WindowRounding = 5.0f;
    style.TabRounding = 0.0f;
    style.ItemSpacing = ImVec2(5, 5);
    style.FramePadding = ImVec2(6, 4);
}
