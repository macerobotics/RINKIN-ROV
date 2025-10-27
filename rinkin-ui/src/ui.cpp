#include <stdio.h>
#include <imgui.h>
#include <rlImGui.h>
#include "ui.h"

void ui() {
    ImGui::Begin("Gamepad");
    if(IsGamepadAvailable(0)) {
        ImGui::Text("%s", GetGamepadName(0));
        for(int i = 0; i < GetGamepadAxisCount(0); i++) {
            ImGui::PushID(i);
            float val = GetGamepadAxisMovement(0, i);
            ImGui::InputFloat("", &val);
            if(i == 3) {

            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}