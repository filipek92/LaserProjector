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

void UART4_IRQHandler(void){
	if(pc_uart.RxState == HAL_UART_STATE_BUSY_RX){
		HAL_UART_IRQHandler(&pc_uart);
		return;
	}

	if((__HAL_UART_GET_FLAG(&pc_uart, UART_FLAG_RXNE) != RESET)
		&& (__HAL_UART_GET_IT_SOURCE(&pc_uart, UART_IT_RXNE) != RESET))
	{
		TERM_ParseByte(&term, pc_uart.Instance->DR);
	}
}

void USART2_IRQHandler(void){
	//HAL_USART_IRQHandler(&pc_usart);
}

void USART3_IRQHandler(void){
	//HAL_USART_IRQHandler(&pc_uart);
}

void TIM2_IRQHandler(void){
	HAL_TIM_IRQHandler(&tim);
}

void TIM3_IRQHandler(void){
	HAL_TIM_IRQHandler(&tim_motor);
}

void DMA1_Stream2_IRQHandler(){
	HAL_DMA_IRQHandler(&dmauartrx);
}
