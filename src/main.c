#include "main.h"
#include "led.h"
#include "init.h"

#include "img.h"

#define CLEAN_START 40 // PreImage data
#define CLEAN_END	30 // AfterImage data
#define BIT_LENGTH	Y_SIZE+((CLEAN_START+CLEAN_END)*8)

void sendLine();
void inc_line();


DMA_HandleTypeDef dmaspitx;
SPI_HandleTypeDef print_spi;
TIM_HandleTypeDef tim;
TIM_HandleTypeDef tim_spi_clock;
UART_HandleTypeDef pc_uart;

volatile uint16_t scan_line = 0;

HAL_StatusTypeDef status;

float line_frequency;
float bit_frequency;

volatile uint16_t diff;
volatile uint8_t flag=0;

uint8_t buffer[100];

void main(){
	init_peripherals();
	printf("Projector Boot\r\n");
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);

	LED_On(LED_G);

	printf("Ready\r\n");
	sendLine();
	scan_line = 3;
	while (1){
		LED_Toggle(LED_G);
		HAL_Delay(10);
		inc_line();
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

		float arr_reg = 42.0/(line_frequency*BIT_LENGTH)*1000000.0 - 1;
		tim_spi_clock.Instance->ARR = arr_reg;

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


void UART_RX_Callback(UART_HandleTypeDef *husart, uint8_t byte){
	UNUSED(husart);
	switch(byte){
	case 'f':
		printf("Print frequency %d HZ\r\n", (int)line_frequency);
		break;
	default:
		printf("Unknown command \"%c\"!\r\n", byte);
		break;
	}
}
