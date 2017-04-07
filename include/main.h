#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/Trace.h"
#include "stm32f4xx.h"
#include "terminal.h"

extern SPI_HandleTypeDef	print_spi;
extern TIM_HandleTypeDef	tim;
extern TIM_HandleTypeDef	tim_motor;
extern UART_HandleTypeDef	pc_uart;
extern DMA_HandleTypeDef	dmaspitx;
extern DMA_HandleTypeDef	dmauartrx;
extern Terminal_t			term;
extern CRC_HandleTypeDef	hcrc;

#endif
