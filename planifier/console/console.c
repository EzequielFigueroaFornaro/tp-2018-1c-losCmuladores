/*
 * console.c
 *
 *  Created on: 7 jun. 2018
 *      Author: utnso
 */

#include "console.h"

command_result execute_command(command command) {
	command_result result;
	if (!valid_args(command)) {
		result.code = INVALID_ARGS;
		result.content = string_from_format("Invalid arguments for command '%s'", command.code_str);
		return result;
	}
	switch(get_command_code(command.code_str)) {
	case LIST:
		return list_cmd(command);
	case PAUSE:
		return pause_cmd(command);
	case RESUME:
		return resume_cmd(command);
	case BLOCK:
		return block_cmd(command);
	case UNBLOCK:
		return unblock_cmd(command);
	case STATUS:
		return status_cmd(command);
	case KILL:
		return kill_cmd(command);
	case DEADLOCK:
		return deadlock_cmd(command);
	case ADD:
		return add_cmd(command);
	case LIST_ESIS:
		return list_esis_cmd(command);
	default:
		result.code = INVALID_COMMAND;
		result.content = string_from_format("Invalid command '%s'", command.code_str);
		return result;
	}
	return result;
}

void listen_for_commands() {
	command command;
	while (!is_exit(command = parse_command(readline("> ")))) {
		if (command.code_str == NULL || string_is_empty(command.code_str)) {
			continue;
		}
		if (!command_exists(command)) {
			print_and_log_error("Invalid command '%s'", command.code_str);
			continue;
		}
		command_result result = execute_command(command);
		if (result.code == COMMAND_OK) {
			print_and_log("%s", result.content);
		} else {
			print_and_log_error("%s", result.content);
		}
		destroy_command(command);
	}
	quit_console("Exiting...", EXIT_SUCCESS);
}

pthread_t start_console() {
	start_signal_listener();
	start_console_log();
	load_commands();

	pthread_t console_thread;
	if (pthread_create(&console_thread, NULL, (void*) listen_for_commands, NULL) < 0) {
		quit_console("Error starting console thread", EXIT_FAILURE);
	};
	return console_thread;
}
