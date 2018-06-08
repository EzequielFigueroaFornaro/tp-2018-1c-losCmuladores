#ifndef CONSOLE_COMMANDS_HANDLER_H_
#define CONSOLE_COMMANDS_HANDLER_H_

typedef struct {
	command command;
	t_list parameters;
}__attribute((packed)) console_command;

typedef enum {
	PAUSE, CONTINUE, BLOCK, UNLOCK, LIST, KILL, STATUS, DEADLOCK, UNKNOWN
} command;

console_command build_console_command(char** raw_command);

#endif
