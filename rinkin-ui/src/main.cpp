#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>
#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>
#include <imgui.h>
#include <rlImGui.h>
#include <implot.h>
#include <lua.hpp>
#include "lua_bindings/lua_bindings.h"
#include "lua_bindings/lua_udp.h"
#include "config.h"
#include "ui.h"
#include "util.h"
#include "windows_fix.h"

#include "lighting.vs.h"
#include "lighting.fs.h"

#define GLSL_VERSION            330

float heading = 0.0f, pitch = 0.0f, roll = 0.0f;
RenderTexture2D model_texture;

void lua_simple_fcall(lua_State *L, const char *fname) {
	lua_getglobal(L, fname);
	if(lua_isfunction(L, -1)) {
		int res = lua_pcall(L, 0, 0, 0)	;
		if(res != LUA_OK)
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
	}
	lua_settop(L, 0);
}

int main(int argc, char* argv[]) {
	#ifdef _WIN32
	windows_networking_init();
	#endif
	lua_State *L = luaL_newstate();
	if(!L) FATAL("failed to initialize Lua");
	luaL_openlibs(L);
	lua_register_bindings(L);
	
	
	int screenWidth = 1280;
	int screenHeight = 800;

	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "Rinkin");
	SetTargetFPS(60);
	rlImGuiSetup(true);
	ImGui::StyleColorsLight();
	ImPlot::CreateContext();
	

	Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 100.0f, -1000.0f };// Camera position perspective
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, -1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 30.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera type

	Shader shader = LoadShaderFromMemory((const char*)lighting_vs, (const char*)lighting_fs);
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

	Light light;
    light = CreateLight(LIGHT_POINT, (Vector3){ 0, 500, 0 }, Vector3Zero(), WHITE, shader);

	Model model = LoadModel("model.obj");

	model_texture = LoadRenderTexture(640, 480);

	for (int i = 0; i < model.materialCount; i++) {
    	model.materials[i].shader = shader;
	}

	int res = luaL_dofile(L, "lua/main.lua");
	if(res != LUA_OK) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_settop(L, 0);
	}

	lua_simple_fcall(L, "setup");

	while (!WindowShouldClose()) {
		lua_udp_callback(L);
		float motor = GetGamepadAxisMovement(0, 3) * -1;
		static float last_motor = 0;
		if(motor != last_motor) {
			char payload[6] = {0};
			sprintf(payload, "%.03f", motor);
			LOG("publish /motor %s", payload);
			//mosquitto_publish(mosq, nullptr, "/motor", strlen(payload) + 1, payload, 0, false);
			last_motor = motor;
		}

		//UpdateCamera(&camera, CAMERA_ORBITAL);
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		model.transform = MatrixRotateXYZ((Vector3){pitch, heading, roll});

		BeginTextureMode(model_texture);
		{
			ClearBackground(DARKGRAY);
			BeginMode3D(camera);
			{
				BeginShaderMode(shader);
				{
					DrawModel(model, (Vector3){0.0f, 0.0f, 0.0f}, 1.0f, WHITE);
					//DrawCube(Vector3Zero(), 50, 50, 50, WHITE);
				}
				EndShaderMode();
				DrawSphereEx(light.position, 10.0f, 8, 8, light.color);
			}
			EndMode3D();
		}
		EndTextureMode();

		BeginDrawing();
		{
			ClearBackground(DARKGRAY);

			DrawFPS(10, 10);

			rlImGuiBegin();

			lua_simple_fcall(L, "loop");

			ui();

			if(ImGui::Begin("Moteurs")) {
				if(ImGui::SliderFloat("Moteur 1", &motor, -1, 1)) {

				}
			}
			ImGui::End();

			//ImGui::ShowDemoWindow();


			rlImGuiEnd();
		}
		EndDrawing();
	}

	UnloadShader(shader);
	UnloadModel(model);
	ImPlot::DestroyContext();
    rlImGuiShutdown();
	CloseWindow();
	lua_close(L);
	#ifdef _WIN32
	windows_networking_cleanup();
	#endif
	return 0;
}
