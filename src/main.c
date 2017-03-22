#include "main.h"
#include "led.h"
#include "init.h"

#include "img.h"

#define CLEAN_START 40 // PreImage data
#define CLEAN_END	30 // AfterImage data
#define BIT_LENGTH	Y_SIZE+((CLEAN_START+CLEAN_END)*8)

void sendLine();
void inc_line();

//Commands
void arglist(int argc, char *argv[]);
void setline(int argc, char *argv[]);
void setrotfreq(int argc, char *argv[]);
void linefreq(int argc, char *argv[]);
void steptoline(int argc, char *argv[]);
void motor(int argc, char *argv[]);


DMA_HandleTypeDef	dmaspitx;
SPI_HandleTypeDef	print_spi;
TIM_HandleTypeDef	tim;
TIM_HandleTypeDef	tim_motor;
UART_HandleTypeDef 	pc_uart;
Terminal_t			term;

volatile uint16_t scan_line = 0;
volatile int steptoline_cnt = 1000;

HAL_StatusTypeDef status;

float line_frequency;
float bit_frequency;

volatile uint16_t diff;
volatile uint8_t flag=0;

uint8_t buffer[100];

void main(){
	init_peripherals();
	TERM_AddAsciiCommand(&term, "args", arglist);
	TERM_AddAsciiCommand(&term, "line", setline);
	TERM_AddAsciiCommand(&term, "rotf", setrotfreq);
	TERM_AddAsciiCommand(&term, "linef", linefreq);
	TERM_AddAsciiCommand(&term, "liner", steptoline);
	TERM_AddAsciiCommand(&term, "motor", motor);


	printf("Projector Boot\r\n");
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);

	LED_On(LED_G);

	printf("Ready\r\n");
	TERM_Prompt(&term);
	sendLine();
	scan_line = 3;
	while (1){
		LED_Toggle(LED_G);
		HAL_Delay(50);
	}
}

void sendLine(){
	uint8_t *data = (uint8_t *) img+(scan_line*Y_SIZEB);

	LED_On(LED_O);
	HAL_SPI_Transmit_DMA(&print_spi, data, Y_SIZEB);
	LED_Off(LED_O);
}

void inc_line(){
	scan_line = (scan_line+1)%X_SIZE;
}

void HAL_GPIO_EXTI_Callback(uint16_t pin){
	if(pin==GPIO_PIN_0) inc_line();
	if(pin==GPIO_PIN_7) sendLine();
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim){
	static uint16_t last_value;
	static uint8_t arr[50];

	if (htim->Instance==TIM2){
		uint16_t now = __HAL_TIM_GET_COMPARE(&tim, TIM_CHANNEL_4);	//read TIM2 channel 1 capture value
		uint16_t scan_time = (now-last_value); //in us;
		line_frequency = 1000000.0/scan_time;
		last_value = now;

		memset(arr, 0xFF, 20);
#if CLEAN_START > 20
		memset(arr+20, 0, CLEAN_START-20);
#endif
		arr[CLEAN_START-2] = 0x33;
		arr[CLEAN_START-1] = 1;
		HAL_SPI_Transmit(&print_spi, arr, CLEAN_START, 2);
		sendLine();
	}
}

void arglist(int argc, char *argv[]){
	printf("ArgList:\r\n");
	for(int i = 0; i < argc; i++){
		printf(" %d. \"%s\"\r\n", i, argv[i]);
	}
}

void setline(int argc, char *argv[]){
	if(argc==2) scan_line = atoi(argv[1]);
	printf("Scanline %d\r\n", scan_line);
}

void setrotfreq(int argc, char *argv[]){
	if(argc==2){
		uint32_t arr_reg = (42000000.0/4)/atof(argv[1]) - 1;
		tim_motor.Instance->ARR = arr_reg;
	}
	float freq = (42000000.0/4)/(tim_motor.Instance->ARR + 1);
	printf("Motor ARR=%lu\r\n", tim_motor.Instance->ARR);
	printf("Motor frequency %lu Hz\r\n", (uint32_t) freq);
}

void linefreq(int argc, char *argv[]){
	UNUSED(argc); UNUSED(argv);
	printf("Scan frequency %lu Hz\r\n", (uint32_t) line_frequency);
}

void steptoline(int argc, char *argv[]){
	if(argc==2) steptoline_cnt = atoi(argv[1]);
	printf("Step to line ratio %d steps/line\r\n", steptoline_cnt);
}

void motor(int argc, char *argv[]){
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, (argc > 1)?GPIO_PIN_SET:GPIO_PIN_RESET);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	static int cnt=0;
	if(htim == &tim_motor){
		if(cnt++ % steptoline_cnt == 0){
			LED_Toggle(LED_B);
			inc_line();
		}
	}
}
