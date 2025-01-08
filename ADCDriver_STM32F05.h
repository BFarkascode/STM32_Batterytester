/*
 *  Created on: Jan 08, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_Batterytester
 *  Processor: STM32F405RG
 *  File: ADCDriver_STM32F05.h
 *  Header version: 1.0
 *  Change history:
 */

#ifndef INC_ADCDRIVER_STM32F05_H_
#define INC_ADCDRIVER_STM32F05_H_

#include "stm32f405xx.h"
#include "main.h"

//LOCAL CONSTANT
const static uint16_t* vrefint_cal_ptr = 0x1FFF7A2A;					//this is the vrefint calibration value pointer (see datasheet p139)

//LOCAL VARIABLE

//EXTERNAL VARIABLE
extern enum_Yes_No_Selector Battery_low;

//FUNCTION PROTOTYPES
void Battery_ADC_Init(void);
float Battery_ADC_Readout(void);
enum_Yes_No_Selector Battery_Eval(void);

#endif /* INC_ADCDRIVER_STM32F05_H_ */
