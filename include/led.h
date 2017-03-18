#ifndef LED_H
#define LED_H

#include "stm32f4xx.h"

#define LED_GPIO GPIOD
#define LED_O GPIO_PIN_13
#define LED_G GPIO_PIN_12
#define LED_R GPIO_PIN_14
#define LED_B GPIO_PIN_15
#define LED_ALL (LED_R|LED_G|LED_B|LED_O)

void LED_Init();

inline void LED_On(uint32_t led){LED_GPIO->ODR |= (led & LED_ALL);}
inline void LED_Off(uint32_t led){LED_GPIO->ODR &= ~(led & LED_ALL);}
inline void LED_Toggle(uint32_t led){LED_GPIO->ODR ^= (led & LED_ALL);}

#endif
