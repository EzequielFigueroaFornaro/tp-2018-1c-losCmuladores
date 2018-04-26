#include "ise.h"

int main(int argc, char* argv[]) {
	load_configuration(argv[1]);

	handshake_planifier();

//	handshake_coordinator();

	return EXIT_SUCCESS;
}

int handshake_planifier() {
	int connect_result = connect_to(planifier_ip, planifier_port);
	if (connect_result < 0) {
		log_error(logger, "Could not connect to planifier on %s:%d",
				planifier_ip, planifier_port);
		exit_gracefully(EXIT_FAILURE);
	}

	int socket = connect_result;
	if (send_module_connected(socket, ESI) < 0) {
		log_error(logger, "Could not send message to planifier on %s:%d", planifier_ip, planifier_port);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}

	message_type message_type;
	int message_type_result = recv(socket, &message_type, sizeof(message_type),	MSG_WAITALL);

	if (message_type_result <= 0 || message_type != CONNECTION_SUCCESS) {
		log_error(logger,
				"Expected CONNECTION_SUCCESS (%d) from planifier in (%s:%d), actual: %d",
				CONNECTION_SUCCESS, planifier_ip, planifier_port, message_type);
		close(socket);
		exit_gracefully(EXIT_FAILURE);
	}
	log_info(logger, "Connected to planifier!");
	return connect_result;
}

void load_configuration(char* config_file_path) {
	configure_logger();
	log_info(logger, "Loading configuration file...");
	t_config* config = config_create(config_file_path);

	planifier_port = config_get_int_value(config, "PLANIFIER_PORT");
	planifier_ip = config_get_string_value(config, "PLANIFIER_IP");

	log_info(logger, "OK.");
}

void configure_logger() {
	logger = log_create("esi.log", "esi", true, LOG_LEVEL_INFO);
}

void exit_gracefully(int code) {
	log_destroy(logger);
	exit(code);
}
