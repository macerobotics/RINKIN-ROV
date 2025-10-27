#include <string>

struct mqtt_message {
	std::string topic, payload;
};

void mqtt_init();
void mqtt_deinit();
void mqtt_connect(const char *host, int port);
void mqtt_subscribe(const char *topic);
void mqtt_publish(const char *topic, const char *payload);
bool mqtt_has_message();
struct mqtt_message mqtt_get_message();

