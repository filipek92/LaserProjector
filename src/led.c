#include "led.h"

void LED_Init(){
	__GPIOD_CLK_ENABLE();
	GPIO_InitTypeDef gpio;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FAST;
	gpio.Pin = LED_ALL;

	HAL_GPIO_Init(GPIOD, &gpio);
}
