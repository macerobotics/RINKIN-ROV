#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#define RLIGHTS_IMPLEMENTATION
#include <rlights.h>
#include <imgui.h>
#include <rlImGui.h>
#include <mosquitto.h>
#include "config.h"
#include "ui.h"
#include "util.h"

#include "lighting.vs.h"
#include "lighting.fs.h"

#define GLSL_VERSION            330

ini_t *config;

#define LAST(x) x[(x##_offset - 1+ IM_ARRAYSIZE(x)) % IM_ARRAYSIZE(x)]
static float heading[256] = {0};
static int heading_offset = 0;
float pitch = 0.0f, roll = 0.0f;

enum {
	DISCONNECTED,
	CONNECTING,
	CONNECTED,
} connection_status;

void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		mosquitto_disconnect(mosq);
	}
	connection_status = CONNECTED;
	rc = mosquitto_subscribe(mosq, NULL, "/imu/heading", 1);
	if(rc != MOSQ_ERR_SUCCESS){
		fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
		mosquitto_disconnect(mosq);
	}
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
	connection_status = DISCONNECTED;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("topic: %s\tpayload:%s\n", msg->topic, (const char*)msg->payload);
	if(!strcmp(msg->topic, "/imu/heading")) {
		heading[heading_offset] = strtof((const char*)msg->payload, NULL);
		heading_offset = (heading_offset + 1) % IM_ARRAYSIZE(heading);
	}
}

int main(int argc, char* argv[]) {
	int screenWidth = 1280;
	int screenHeight = 800;
	const char *ip_address = nullptr;

	config = ini_load("config.ini");
	if(!config)
		FATAL("failed to open config.ini");
	
	ip_address = ini_get(config, "raspberry-pi", "ip-address");
	if(!ip_address)
		FATAL("ip address not found");

	struct mosquitto *mosq;
	int rc = mosquitto_lib_init();
	if(rc != MOSQ_ERR_SUCCESS)
		FATAL("failed to initialize mosquitto: %s", mosquitto_strerror(rc));

	mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL)
		FATAL("out of memory");

	mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_message_callback_set(mosq, on_message);
	LOG("connecting to MQTT server");
	connection_status = CONNECTING;
	rc = mosquitto_connect_async(mosq, ip_address, 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS) {
		mosquitto_destroy(mosq);
		FATAL("mosquitto error: %s\n", mosquitto_strerror(rc));
	}

	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS) {
		mosquitto_destroy(mosq);
		FATAL("failed to start mosquitto loop: %s\n", mosquitto_strerror(rc));
	}

	SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
	InitWindow(screenWidth, screenHeight, "ROV");
	SetTargetFPS(60);
	rlImGuiSetup(true);

	Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 100.0f, -1000.0f };// Camera position perspective
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 30.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera type

	Shader shader = LoadShaderFromMemory((const char*)lighting_vs, (const char*)lighting_fs);
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.1f, 0.1f, 0.1f, 1.0f }, SHADER_UNIFORM_VEC4);

	Light light;
    light = CreateLight(LIGHT_POINT, (Vector3){ 0, 500, 0 }, Vector3Zero(), WHITE, shader);

	Model model = LoadModel("model.obj");

	for (int i = 0; i < model.materialCount; i++) {
    	model.materials[i].shader = shader;
	}

	while (!WindowShouldClose()) {
		float motor = GetGamepadAxisMovement(0, 3) * -1;
		static float last_motor = 0;
		if(motor != last_motor) {
			char payload[6] = {0};
			sprintf(payload, "%.03f", motor);
			LOG("publish /motor %s", payload);
			mosquitto_publish(mosq, nullptr, "/motor", strlen(payload) + 1, payload, 0, false);
			last_motor = motor;
		}

		//UpdateCamera(&camera, CAMERA_ORBITAL);
		float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

		model.transform = MatrixRotateXYZ((Vector3){DEG2RAD * pitch, DEG2RAD * (LAST(heading) - 90.0f), DEG2RAD * roll});
		BeginDrawing();
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

			rlImGuiBegin();

			ui();

			if(ImGui::Begin("IMU")) {
				ImGui::PlotLines("heading", heading, IM_ARRAYSIZE(heading), heading_offset, nullptr, 0.0f, 360.0f, ImVec2(0, 80.0f));
			}
			ImGui::End();

			if(connection_status != CONNECTED) {
				if(ImGui::Begin("MQTT")) {
					if(connection_status == CONNECTING)
						ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime(), ImVec2(0.0f, 0.0f), "Connexion en cours...");
					else
						ImGui::Text("disconnected");
				}
				ImGui::End();
			}


			ImGui::ShowDemoWindow();


			rlImGuiEnd();
		}
		EndDrawing();
	}

	mosquitto_lib_cleanup();
	UnloadShader(shader);
	UnloadModel(model);
    rlImGuiShutdown();
	CloseWindow();
	ini_free(config);
	return 0;
}
