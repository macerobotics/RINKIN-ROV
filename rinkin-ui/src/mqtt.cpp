#include <string.h>
#include <mosquitto.h>
#include <string>
#include <queue>
#include <unordered_set>
#include <mutex>
#include "mqtt.h"
#include "util.h"

static enum {
    DISCONNECTED,
	CONNECTING,
	CONNECTED,
} connection_status;

static struct mosquitto *mosq;

static std::queue<mqtt_message> mqtt_messages;
static std::mutex queue_mutex;

static std::unordered_set<std::string> subscriptions;
static std::mutex subscriptions_mutex;

static void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
	int rc;
	printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
	if(reason_code != 0){
		mosquitto_disconnect(mosq);
	}
	connection_status = CONNECTED;
	subscriptions_mutex.lock();
	for(auto subscription: subscriptions) {
		rc = mosquitto_subscribe(mosq, NULL, subscription.c_str(), 1);
		LOG("subscribing to %s", subscription.c_str());
		if(rc != MOSQ_ERR_SUCCESS){
			fprintf(stderr, "Error subscribing: %s\n", mosquitto_strerror(rc));
			mosquitto_disconnect(mosq);
		}
	}
	subscriptions_mutex.unlock();
}

static void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
	connection_status = DISCONNECTED;
}

static void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
	printf("topic: %s\tpayload:%s\n", msg->topic, (const char*)msg->payload);
	queue_mutex.lock();
	struct mqtt_message queued_msg = {std::string(msg->topic), std::string((const char*)msg->payload)};
	mqtt_messages.push(queued_msg);
	queue_mutex.unlock();
	//if(!strcmp(msg->topic, "/imu/heading")) {
	//	heading[heading_offset] = strtof((const char*)msg->payload, NULL);
	//	heading_offset = (heading_offset + 1) % ARRAYSIZE(heading);
	//}
}

void mqtt_init() {
    mosq = mosquitto_new(NULL, true, NULL);
	if(mosq == NULL) FATAL("out of memory");
    int rc = mosquitto_lib_init();
	if(rc != MOSQ_ERR_SUCCESS)
		FATAL("failed to initialize mosquitto: %s", mosquitto_strerror(rc));
    mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_disconnect_callback_set(mosq, on_disconnect);
	mosquitto_message_callback_set(mosq, on_message);
}

void mqtt_deinit() {
    mosquitto_lib_cleanup();
}

void mqtt_connect(const char *host, int port) {
    LOG("connecting to MQTT server %s:%d", host, port);
	connection_status = CONNECTING;
	int rc = mosquitto_connect_async(mosq, host, port, 60);
	if(rc != MOSQ_ERR_SUCCESS) {
		mosquitto_destroy(mosq);
		FATAL("mosquitto error: %s\n", mosquitto_strerror(rc));
	}

	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS) {
		mosquitto_destroy(mosq);
		FATAL("failed to start mosquitto loop: %s\n", mosquitto_strerror(rc));
	}
}

void mqtt_subscribe(const char *topic) {
	subscriptions_mutex.lock();
	subscriptions.insert(std::string(topic));
	subscriptions_mutex.unlock();
}

void mqtt_publish(const char *topic, const char *payload) {
	mosquitto_publish(mosq, nullptr, "/motor", strlen(payload) + 1, payload, 0, false);
}

bool mqtt_has_message() {
	queue_mutex.lock();
	bool r = !mqtt_messages.empty();
	queue_mutex.unlock();
	return r;
}

struct mqtt_message mqtt_get_message() {
	queue_mutex.lock();
	struct mqtt_message r = mqtt_messages.front();
	mqtt_messages.pop();
	queue_mutex.unlock();
	return r;
}