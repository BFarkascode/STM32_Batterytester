/*
 *  Created on: Jan 08, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_Batterytester
 *  Processor: STM32F405RG
 *  File: ClockDriver_STM32F405.h
 *  Header version: 1.0
 *  Change history:
 */

#ifndef INC_RCCTIMPWMDELAY_CUSTOM_H_
#define INC_RCCTIMPWMDELAY_CUSTOM_H_

#include "stdint.h"
#include "stm32f405xx.h"

//LOCAL CONSTANT

//LOCAL VARIABLE

//EXTERNAL VARIABLE

//FUNCTION PROTOTYPES
void SysClockConfig(void);
void TIM6Config (void);
void Delay_us(int micro_sec);
void Delay_ms(int milli_sec);
void TIM7Config (void);

#endif /* RCCTIMPWMDELAY_CUSTOM_H_ */
