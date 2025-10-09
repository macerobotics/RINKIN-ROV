#include <stdio.h>
#include <imgui.h>
#include <rlImGui.h>
#include <alloca.h>
#include "config.h"
#include "ui.h"
#include "video.h"


static Video *video = nullptr;
const unsigned int width = 640, height = 480;

void ui() {
    if(!video) {
        //video = new Video("http://www.windsurfbreizh22.com/webcamHD/webcam-rosaires/video.php", width, height);
        const char *ip_addr = ini_get(config, "raspberry-pi", "ip-address");
        if(!ip_addr) {
            ImGui::Text("ip address not found in the config file");
            return;
        }
        char *url = (char*)alloca(strlen(ip_addr) + sizeof("rtsp://" ":8554/cam"));
        sprintf(url, "rtsp://%s:8554/cam", ip_addr);
        video = new Video(url, width, height);
    }

    static bool err = false;
    ImGui::Begin("Video");
    if(ImGui::Button("start")) {
        if(!video->start()) {
            err = true;
        }
    }
    if(err) ImGui::Text("failed to start video");
    if(ImGui::Button("stop"))
        video->stop();
    //ImGui::Image(video->acquire_texture(), ImVec2(width, height));
    rlImGuiImage((const Texture*)video->get_texture());
    ImGui::End();

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