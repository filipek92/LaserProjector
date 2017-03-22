#ifndef TERMINAL_H
#define TERMINAL_H

#define TERMINAL_CMD_COUNT		30
#define TERMINAL_ARGS_COUNT		10
#define TERMINAL_BUFFER_LENGTH	50

#define TERMINAL_MODE_ASCII		0
#define TERMINAL_MODE_BINARY	1

#define TERMINAL_ECHO_ENABLE	1
#define TERMINAL_ECHO_DISABLE	0

#define TERMINAL_CMD_NOT_FOUND	-1
#define TERMINAL_CMD_HELP		-2

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef union {
	void (*ascii)(int, char **);
	void (*binnary)(int, uint8_t *);
} Terminal_Callback_t;

typedef struct {
	char *name;
	Terminal_Callback_t callback;
	uint8_t mode;
} Terminal_Command_t;

typedef struct {
	Terminal_Command_t commands[TERMINAL_CMD_COUNT];
	char buffer[TERMINAL_BUFFER_LENGTH];
	char *prompt;
	uint8_t pointer;
	uint8_t cmd_cnt;
	uint8_t mode;
	uint8_t last;
	uint8_t echo;
	uint8_t esc_char;
} Terminal_t;

int TERM_ParseByte(Terminal_t *term, char byte);
void TERM_ParseLine(Terminal_t *term, char line[]);
int TERM_CallCommand(Terminal_t *term, const char name[], int argc, char **argv);
void TERM_Prompt(Terminal_t *term);

void TERM_AddAsciiCommand(Terminal_t *term, char *name, void (*callback)(int argc, char **argv));
void TERM_AddBinaryCommand(Terminal_t *term, char *name, void (*callback)(int len, uint8_t *data));

void TERM_Help(Terminal_t *term);


#endif
