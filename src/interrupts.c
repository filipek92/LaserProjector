#include "stm32f4xx.h"
#include "main.h"

void SysTick_Handler(void) {
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

void USART2_IRQHandler(void){
	HAL_USART_IRQHandler(&pc_usart);
}

void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&tim);
}

void DMA1_Stream0_IRQHandler(){
	HAL_DMA_IRQHandler(&dmaspitx);
}
