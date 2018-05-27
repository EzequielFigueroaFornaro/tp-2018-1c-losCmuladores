#include "config.h"

t_module_config* coordinator_config;
t_module_config* planifier_config;

void load_config(char* config_file_path, int coordinator, int planifier) {
	log_info(logger, "Loading configuration file...");

	t_config* config = config_create(config_file_path);

	coordinator_config = malloc(sizeof(t_module_config));
	coordinator_config->ip = config_get_string_value(config, "COORDINATOR_IP");
	coordinator_config->port = config_get_int_value(config, "COORDINATOR_PORT");

	planifier_config = malloc(sizeof(t_module_config));
	planifier_config->ip = config_get_string_value(config, "PLANIFIER_IP");
	planifier_config->port = config_get_int_value(config, "PLANIFIER_PORT");

	modules_config = dictionary_create();
	dictionary_put(modules_config, string_itoa(coordinator), coordinator_config);
	dictionary_put(modules_config, string_itoa(planifier), planifier_config);

//	config_destroy(config);
	log_info(logger, "OK Loading.");
}

char* get_ip(int module) {
	return get_config(module)->ip;
}

int get_port(int module) {
	return get_config(module)->port;
}

int get_socket(int module) {
	return get_config(module)->socket;
}

t_module_config* get_config(int module) {
	return dictionary_get(modules_config, string_itoa(module));
}

void destroy_config() {
	free(coordinator_config);
	free(planifier_config);
	dictionary_destroy(modules_config);
}
