#include "stm32f4xx.h"
#include "main.h"
#include "led.h"

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_7);
}

void USART2_IRQHandler(void){
	HAL_USART_IRQHandler(&pc_usart);
}

void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&tim);
}

void DMA2_Stream3_IRQHandler(){
	static uint8_t FULL = 0xFF;
	HAL_DMA_IRQHandler(&dmaspitx);
	HAL_SPI_Transmit(&print_spi, &FULL, 1, 1); //Send Dummy 0xFF, to force output to 1 for synchro impuls
}
