#include "terminal.h"

void TERM_PrintBuffer(Terminal_t *term);

int is_white_char(char ch){
	return (ch == ' ') | (ch == '\t');
}

int is_end_line(char ch){
	return (ch == '\r') | (ch == '\n');
}

int is_backspace(char ch){
	return (ch == 127) | (ch == 8);
}

int TERM_BinaryParse(Terminal_t *term, uint8_t byte){
	(void) term;
	(void) byte;
	return 0;
}

int TERM_ParseByte(Terminal_t *term, char byte){
	char last = term->last;
	term->last = byte;

	if(byte==term->esc_char && last!=term->esc_char) return 0; //Maybe escape
//
	if(last==term->esc_char && byte!=term->esc_char){ //Escaped sequence
		printf("\r\nEscaped by 0x%x, ", term->esc_char);
		printf("CMD == 0x%x\r\n", byte);
	}
//
//	if(term->mode == TERMINAL_MODE_BINARY) return TERM_BinaryParse(term,(uint8_t) byte); //If binary -> redirect
//
	if(is_backspace(byte) && term->pointer==0) return 0; // Buffer empty, backspace has no mean
//
	if(term->echo) _write(1, &byte, 1);//putchar(byte); // Echo if enabled
	if(term->echo && (byte=='\r')) _write(1, "\n", 1);

	if(is_backspace(byte)){
		term->pointer--; // Backspace
		return 0;
	}

	if((byte=='\n') && (last =='\r')){
		return 0;
	}

	if(is_end_line(byte)){ //End of line
		term->buffer[term->pointer] = 0;
		TERM_ParseLine(term, term->buffer);
		term->pointer = 0;
		TERM_Prompt(term);
		return 1;
	}
	term->buffer[term->pointer++] = byte;
	if(term->pointer == TERMINAL_BUFFER_LENGTH){
		term->pointer = 0;
		printf("\r\nCMD too long\r\n");
		TERM_Prompt(term);
	}
	return 0;
}

void TERM_PrintBuffer(Terminal_t *term){
	term->buffer[term->pointer] = 0;
	printf("Buffer \"%s\"\r\n", term->buffer);
}

void TERM_ParseLine(Terminal_t *term, char line[]){
	char *argv[TERMINAL_ARGS_COUNT];
	char *p = line;
	argv[0] = line;
	int argc = 1;
	uint8_t arg = 1;

	if(!*p){
		return;
	}

	while((*(++p) != 0) && (argc <= TERMINAL_ARGS_COUNT)){
		if(is_white_char(*p)){
			arg = 0;
			*p = 0;
		}else{
			if(!arg){
				arg = 1;
				argv[argc] = p;
				argc++;
			}
		}
	}
	if(TERM_CallCommand(term, argv[0], argc, argv)==TERMINAL_CMD_NOT_FOUND){
		printf("Command \"%s\" not found. Use \"help\"\r\n", argv[0]);
	}
}

void TERM_Prompt(Terminal_t *term){
	if(term->prompt) _write(1, term->prompt, strlen(term->prompt));
}

void TERM_AddAsciiCommand(Terminal_t *term, char *name, void (*callback)(int argc, char *argv[])){
	Terminal_Command_t *cmd = &(term->commands[term->cmd_cnt]);
	cmd->callback.ascii = callback;
	cmd->name = name;
	cmd->mode = TERMINAL_MODE_ASCII;
	term->cmd_cnt++;
}

void TERM_AddBinaryCommand(Terminal_t *term, char *name, void (*callback)(int len, uint8_t *data)){
	Terminal_Command_t *cmd = &(term->commands[term->cmd_cnt]);
	cmd->callback.binnary = callback;
	cmd->name = name;
	cmd->mode = TERMINAL_MODE_ASCII;
	term->cmd_cnt++;
}

int TERM_CallCommand(Terminal_t *term, const char name[], int argc, char **argv){
	for(int i=0; i<term->cmd_cnt; i++){
		Terminal_Command_t *cmd = &(term->commands[i]);
		if(cmd->mode != TERMINAL_MODE_ASCII) continue;
		if(strcmp(name, cmd->name)!=0) continue;
		cmd->callback.ascii(argc, argv);
		return i;
	}
	if(strcmp("help", name)==0){
		TERM_Help(term);
		return TERMINAL_CMD_HELP;
	}
	return TERMINAL_CMD_NOT_FOUND;
}

void TERM_Help(Terminal_t *term){
	printf("Available commands:\r\n");
	for(int i=0; i<term->cmd_cnt; i++){
		printf("  %s\r\n", term->commands[i].name);
	}
	printf("  help\r\n");
}
