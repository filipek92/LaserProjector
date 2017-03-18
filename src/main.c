#include "main.h"
#include "led.h"

#include "img.h"

void irq_init();
void spi_init();
void sendLine();
void initLineRate();
void initUsart();

uint16_t line = 0;

DMA_HandleTypeDef dmaspitx;
SPI_HandleTypeDef print_spi;
TIM_HandleTypeDef tim;
USART_HandleTypeDef pc_usart;
uint32_t i=0;

volatile uint8_t start=0;

void main(){
	printf("Projektor Boot\r\n");
	printf("SystemCoreClock: %lu Hz\r\n", SystemCoreClock);

	irq_init();
	spi_init();
	initUsart();
	LED_Init();
	LED_On(LED_G);
	initLineRate();

	printf("Ready\r\n");
	uint16_t last=0;
	while (1){
		uint16_t diff = tim.Instance->CNT - last;
		last = tim.Instance->CNT;
		//printf("Time: %u cnt, %c\r\n", diff, (GPIOB->IDR & GPIO_PIN_11)?'H':'L');
		if(start){
			start = 0;
			sendLine();
		}
		HAL_Delay(100);
	}
}

void sendLine(){
	HAL_StatusTypeDef status;
	uint8_t sel = line%5 != 4;
	LED_Off(LED_ALL);
	LED_On(sel?LED_O:LED_G);

	uint8_t *data = (uint8_t *) img+(line*Y_SIZEB);

	if(sel){
		status = HAL_SPI_Transmit(&print_spi, data, Y_SIZEB, 1000);
		printf("Direct scan over line %d done. %s, %d\r\n", line, (status==HAL_OK)?"OK":"OTHER", status);
	}else{
		status = HAL_SPI_Transmit_DMA(&print_spi, data, Y_SIZEB);
		printf("DMA scan over line %d done. %s, %d\r\n", line, (status==HAL_OK)?"OK":"OTHER", status);
	}
	line = (line+1)%X_SIZE;
}

void spi_init(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF5_SPI1;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pin = GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);

	__HAL_RCC_SPI1_CLK_ENABLE();
	print_spi.Instance = SPI1;
	print_spi.Init.Mode = SPI_MODE_MASTER;
	print_spi.Init.Direction = SPI_DIRECTION_2LINES;
	print_spi.Init.DataSize = SPI_DATASIZE_8BIT;
	print_spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
	print_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	print_spi.Init.NSS = SPI_NSS_SOFT;
	print_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
	print_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	print_spi.Init.TIMode = SPI_TIMODE_DISABLED;
	print_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
	HAL_SPI_Init(&print_spi);

	__HAL_RCC_DMA2_CLK_ENABLE();
	dmaspitx.Init.Channel = DMA_CHANNEL_3;
	dmaspitx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	dmaspitx.Init.PeriphInc = DMA_PINC_DISABLE;
	dmaspitx.Init.MemInc = DMA_MINC_ENABLE;
	dmaspitx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	dmaspitx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	dmaspitx.Init.Mode = DMA_NORMAL;
	dmaspitx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
	dmaspitx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	dmaspitx.Instance = DMA2_Stream3;
	HAL_DMA_Init(&dmaspitx);
	//NVIC_EnableIRQ(DMA2_Stream3_IRQn);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	trace_printf("IRQ on %d\n", GPIO_Pin);
	start = 1;
}


inline void initLineRate(){
	__GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF1_TIM2;
	gpio.Mode = GPIO_MODE_INPUT;
	gpio.Pin = GPIO_PIN_11;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &gpio);

	__TIM2_CLK_ENABLE();
	tim.Instance = TIM2;
	tim.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
	tim.Init.CounterMode = TIM_COUNTERMODE_UP;
	tim.Init.Period = 1000;
	tim.Init.Prescaler = 83;
	tim.Init.RepetitionCounter = 1;
	HAL_TIM_IC_Init(&tim);

	TIM_IC_InitTypeDef ic;
	ic.ICFilter = 0;
	ic.ICPolarity = TIM_ICPOLARITY_FALLING;
	ic.ICPrescaler = TIM_ICPSC_DIV1;
	ic.ICSelection = TIM_ICSELECTION_DIRECTTI;
	HAL_TIM_IC_ConfigChannel(&tim, &ic, TIM_CHANNEL_4);

	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 0);

	HAL_TIM_IC_Start_IT(&tim, TIM_CHANNEL_4);

	HAL_TIM_Base_Init(&tim);
	HAL_TIM_Base_Start_IT(&tim);
}

void initUsart(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Alternate = GPIO_AF7_USART2;
	gpio.Mode = GPIO_MODE_AF_PP;
	gpio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &gpio);

	__HAL_RCC_USART2_CLK_ENABLE();
	pc_usart.Instance = USART2;
	pc_usart.Init.BaudRate = 115200;
	pc_usart.Init.Mode = USART_MODE_TX_RX;
	pc_usart.Init.Parity = USART_PARITY_NONE;
	pc_usart.Init.StopBits = USART_STOPBITS_1;;
	pc_usart.Init.WordLength = USART_WORDLENGTH_8B;
	HAL_USART_Init(&pc_usart);

	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_SetPriority(USART2_IRQn, 0);
}


void irq_init(){
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_IT_FALLING;
	gpio.Pin = GPIO_PIN_0;
	gpio.Speed = GPIO_SPEED_FAST;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio);
	NVIC_SetPriority(EXTI0_IRQn, 0);
	NVIC_EnableIRQ(EXTI0_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	static int i=0;
    //printf("TIM BASE %d\r\n", i++);
    LED_Toggle(LED_B);
}
