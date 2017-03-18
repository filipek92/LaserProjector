#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "stm32f4xx.h"
#include "led.h"

void irq_init();
void spi_init();

SPI_HandleTypeDef print_spi;
uint32_t i;

volatile uint8_t start=0;

void main(){
	trace_puts("Projektor Boot");
	trace_printf("SystemCoreClock: %d Hz\n", SystemCoreClock);

	irq_init();
	spi_init();
	LED_Init();
	LED_On(LED_G);

	while (1){
		if(start){
			start = 0;
			LED_Off(LED_G);
			LED_On(LED_R);
		}
		trace_printf("T = %d s\n", i++);
		HAL_Delay(1000);
	}
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

void spi_init(){
	__HAL_RCC_SPI1_CLK_ENABLE();
	print_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	print_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
	print_spi.Init.CLKPolarity = SPI_POLARITY_HIGH;
	print_spi.Init.DataSize = SPI_DATASIZE_8BIT;
	print_spi.Init.Direction = SPI_DIRECTION_1LINE;
	print_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	print_spi.Init.Mode = SPI_MODE_MASTER;
	print_spi.Init.NSS = SPI_NSS_SOFT;
	print_spi.Init.TIMode = SPI_TIMODE_DISABLED;
	print_spi.Instance = SPI1;
	HAL_SPI_Init(&print_spi);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	trace_printf("IRQ on %d\n", GPIO_Pin);
	start = 1;
}
