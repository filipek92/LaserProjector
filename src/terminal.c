#include "terminal.h"

void TERM_PrintBuffer(Terminal_t *term);
void TERM_EscapeSequence(Terminal_t *term, uint8_t byte);
int TERM_Shortcut(Terminal_t *term, uint16_t shortcut);
int TERM_TabHelp(Terminal_t *term);

inline void writech(char ch){
	write(1, &ch, 1);
}

inline int is_white_char(char ch){
	return (ch == ' ') | (ch == '\t');
}

inline int is_end_line(char ch){
	return (ch == '\r') | (ch == '\n');
}

inline int is_backspace(char ch){
	return (ch == 127) | (ch == 8);
}

int TERM_ControlKeys(int16_t byte){
	if(TERM_Shortcut(NULL, 0x5b00 | byte)) return TERMINAL_BINARY_END_PROMPT;
	return TERMINAL_BINARY_END_NOPROMPT;
}

void TERM_EscapeSequence(Terminal_t *term, uint8_t byte){
	if(byte == 0x5b){
		term->mode = TERMINAL_MODE_BINARY;
		term->bin_callback = TERM_ControlKeys;
		return;
	}

	// Unknown escape -> redirect to shortcut
	uint16_t shortcut = (term->esc_char << 8) | byte;
	if(TERM_Shortcut(term, shortcut)){
		TERM_Prompt(term);
		TERM_PrintBuffer(term);
	}
}

int TERM_Shortcut(Terminal_t *term, uint16_t shortcut){
	if(shortcut == 'I'-'A'+1){
		return TERM_TabHelp(term);
	}

	if(shortcut>0 && shortcut<27){
		printf("\r\nPressed CTRL+%c\r\n", shortcut+'A'-1);
		return 1;
	}

	switch(shortcut){
		case 0x5b41:
			printf("\r\nUP\r\n");
			return 1;
		case 0x5b42:
			printf("\r\nDOWN\r\n");
			return 1;
		case 0x5b43:
			printf("\r\nRIGHT\r\n");
			return 1;
		case 0x5b44:
			printf("\r\nLEFT\r\n");
			return 1;
		default:
			printf("\r\nUnknown shortcut %d (0x%x)\r\n", shortcut, shortcut);
			return 1;
	}
}

int TERM_TabHelp(Terminal_t *term){
	int count = 0;
	int id=0;
	for(int i=0; i<term->cmd_cnt; i++){
		if(strncmp(term->buffer, term->commands[i].name, term->pointer)==0){
			count++;
			id = i;
		}
	}
	if(count==0){
		writech('\a');
		return 0;
	}else if(count == 1){
		int len = strlen(term->commands[id].name);
		int update = len - term->pointer;
		strcpy(term->buffer, term->commands[id].name);
		term->pointer = len;

		write(1, term->buffer+(len-update), update);
		return 0;
	}else{
		printf("\r\nOptions:\r\n");
		for(int i=0; i<term->cmd_cnt; i++){
			printf("  %s\r\n", term->commands[i].name);
		}
		writech('\a');
		return 1;
	}
}

int TERM_ParseByte(Terminal_t *term, char byte){
	char last = term->last;
	term->last = byte;

	if(byte==term->esc_char && last!=term->esc_char) return 0; //Maybe escape
	if(last==term->esc_char && byte!=term->esc_char){ //Escaped sequence
		TERM_EscapeSequence(term, byte);
		return 0;
	}

	if(term->mode == TERMINAL_MODE_BINARY){
		int ret = 0;
		if(term->bin_callback){
			// If callback return 0 -> revert to ASCII mode
			ret = term->bin_callback(byte);
		}
		if(ret == TERMINAL_BINARY_CONTINUE) return 0;
		if(ret == TERMINAL_BINARY_END_PROMPT){
			TERM_Prompt(term);
			TERM_PrintBuffer(term);

			term->mode = TERMINAL_MODE_ASCII;
			term->bin_callback = 0;
		}
		return 1;
	}

	if(is_backspace(byte) && term->pointer==0){// Buffer empty, backspace has no mean
		writech('\a');
		return 0;
	}

	if(term->echo && (isprint(byte) || is_end_line(byte) || is_backspace(byte))) write(1, &byte, 1);// Echo if enabled
	if(term->echo && (byte=='\r')) write(1, "\n", 1);

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
	if(isprint(byte)){
		term->buffer[term->pointer++] = byte;
	}else{
		if(TERM_Shortcut(term, byte)){
			TERM_Prompt(term);
			TERM_PrintBuffer(term);
		}
		return 0;
	}
	if(term->pointer == TERMINAL_BUFFER_LENGTH){
		term->pointer = 0;
		printf("\r\nCMD too long\r\n");
		TERM_Prompt(term);
	}
	return 0;
}

void TERM_PrintBuffer(Terminal_t *term){
	term->buffer[term->pointer] = 0;
	write(1, term->buffer, strlen(term->buffer));
}

void TERM_ParseLine(Terminal_t *term, char line[]){
	char *argv[TERMINAL_BUFFER_LENGTH];
	char *p = line;
	argv[0] = line;
	int argc = 1;
	uint8_t arg = 1;
	uint8_t escape = 0;

	if(!*p){
		return;
	}

	while((*(++p) != 0) && (argc <= TERMINAL_BUFFER_LENGTH)){
		if(*p == '"'){
			escape = !escape;
			p++;
		}
		if(is_white_char(*p) & !escape){
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
	if(term->prompt) write(1, term->prompt, strlen(term->prompt));
}

void TERM_AddCommand(Terminal_t *term, char *name, int (*callback)(int argc, char *argv[])){
	Terminal_Command_t *cmd = &(term->commands[term->cmd_cnt]);
	cmd->callback = callback;
	cmd->name = name;
	term->cmd_cnt++;
}

int TERM_CallCommand(Terminal_t *term, const char name[], int argc, char **argv){
	for(int i=0; i<term->cmd_cnt; i++){
		Terminal_Command_t *cmd = &(term->commands[i]);
		if(strcmp(name, cmd->name)!=0) continue;
		int ret = cmd->callback(argc, argv);
		if(ret){
			printf("Return code is %d\r\n", ret);
		}
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
