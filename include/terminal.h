#ifndef TERMINAL_H
#define TERMINAL_H

#define TERMINAL_CMD_COUNT			30
#define TERMINAL_BUFFER_LENGTH		50

#define TERMINAL_MODE_ASCII			0
#define TERMINAL_MODE_BINARY		1

#define TERMINAL_ECHO_ENABLE		1
#define TERMINAL_ECHO_DISABLE		0

#define TERMINAL_CMD_NOT_FOUND		-1
#define TERMINAL_CMD_HELP			-2

#define TERMINAL_BINARY_CONTINUE	 1
#define TERMINAL_BINARY_END_NOPROMPT 0
#define TERMINAL_BINARY_END_PROMPT	-1

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef int (*Terminal_Callback_t)(int, char **);

typedef struct {
	char *name;
	Terminal_Callback_t callback;
} Terminal_Command_t;

typedef struct {
	Terminal_Command_t commands[TERMINAL_CMD_COUNT];
	int (*bin_callback)(int16_t);
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
void TERM_PrintBuffer(Terminal_t *term);
void TERM_AddCommand(Terminal_t *term, char *name, int (*callback)(int argc, char **argv));

void TERM_Help(Terminal_t *term);


#endif
