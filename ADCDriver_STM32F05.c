/*
 *  Created on: Jan 08, 2025
 *  Author: BalazsFarkas
 *  Project: STM32_Batterytester
 *  Processor: STM32F405RG
 *  File: ADCDriver_STM32F05.c
 *  Program version: 1.0
 *  Change history:
 *
 */

#include "ADCDriver_STM32F05.h"

//1)We set up the basic driver for the ADC
void Battery_ADC_Init(void){
	/*
	 * ADC init to check voltage levels on the BAT pin.
	 * Battery is connected to PA3 through a 100k/100k resistor bridge.
	 * PA3 is ADC1_IN3.
	 * Vrefint is ADC1_IN17. Shortest sampling time should be at minimum 10 us or more.
	 * Vrefint calibration data address is from 0x1FFF7A2A to 0x1FFF7A2B
	 * See here: https://community.st.com/t5/stm32-mcus/how-to-use-the-stm32-adc-s-internal-reference-voltage/ta-p/621425
	 *
	 */

	RCC->AHB1ENR |= (1<<0);										//PORTA clocking

	//1)PA3
	GPIOA->MODER |= (1<<6);										//PA3 analog mode
	GPIOA->MODER |= (1<<7);										//PA3 analog mode
																//we use push-pull

	GPIOA->OSPEEDR |= (1<<6);									//very high speed
	GPIOA->OSPEEDR |= (1<<7);
																//no pull resistor
	//2)Clocking
	RCC->APB2ENR |= (1<<8);										//enable the ADC1 clocking
																//we have 12-bit on the ADC

	ADC1->CR2 &= (1<<1);										//no continuous mode
	ADC1->CR2 |= (1<<10);										//we enable the EOC bit to go HIGH after conversion
																//data is right aligned
																//no DMA

	ADC->CCR |= (1<<23);										//we enable the vrefint channel

	ADC1->SQR1 &= ~(15<<20);									//we want one conversion

	ADC1->SMPR2 |= (7<<9);										//we keep as high of SMPR as possible to cope with a variety of sensor input
																//we are using IN3
	ADC1->SMPR1 |= (7<<21);										//we keep as high of SMPR as possible to cope with a variety of sensor input
																//we are using IN17
}


//2)External analog reading from a single channel
float Battery_ADC_Readout(void) {
	/*
	 * Below we are reading a single channel ADC value to first check the Vdda value, then check the voltage level of the battery attached to the Feather.
	 * Vref of the ADC is Vdda in the Feather - the two are internally connected (not all MCUs are like that!)
	 * Note: since the ADC is voltage sensitive, it will not always detect the right voltage on the battery when the battery is low (ADC reference voltage will not stay at 3.3)
	 * 		 what happens is that as the battery runs low and the reference voltage/Vdda gets messed up, thus the detected voltage value will actually increase when using an arbitrary number in the calculations
	 *
	 */

	//get vrefint
	int32_t vrefint_channel_input;

	ADC1->SQR3 &= ~(31<<0);												//wipe register value
	ADC1->SQR3 |= (17<<0);												//we want the 17th channel as the first conversion

	ADC1->CR2 |= (1<<0);												//we enable the the ADC
	ADC1->CR2 |= (1<<30);												//we start conversion
	while((ADC1->SR & (1<<1)) == 0);									//when the EOC goes HIGH, we go on
																		//we need to wait until the ADC is stabilised and the readout is done
	vrefint_channel_input = ADC1->DR;									//this reads out the data and clears the EOC flag

	ADC1->CR2 &= ~(1<<0);												//we disable the the ADC

	float vdda_val = (3.3 * (*vrefint_cal_ptr)) / vrefint_channel_input ;

	//get the battery voltage on CH3
	int32_t CH3_channel_input;

	ADC1->SQR3 &= ~(31<<0);												//wipe register value
	ADC1->SQR3 |= (3<<0);												//we want the 17th channel as the first conversion

	ADC1->CR2 |= (1<<0);												//we enable the the ADC
	ADC1->CR2 |= (1<<30);												//we start conversion
	while((ADC1->SR & (1<<1)) == 0);									//when the EOC goes HIGH, we go on
																		//we need to wait until the ADC is stabilised and the readout is done
	CH3_channel_input = ADC1->DR;										//this reads out the data and clears the EOC flag

	ADC1->CR2 &= ~(1<<0);												//we disable the the ADC

	float battery_voltage = CH3_channel_input * 2 * (vdda_val / 4096 );	//we calculate the voltage level
																		//Note: as battery voltage drops, the Vdda will not stay at 3V..thus the Vrefint calculations above!
																		//The "2" multiplier is the resistor bridge undone (ADC can't measure more voltage than the reference voltage)
																		//"4096" is 12 bits.
																		//the voltage will be a floating point value

	return battery_voltage;												//we return the raw readout value
}


//3)Battery evaluation
enum_Yes_No_Selector Battery_Eval(void){
	/*
	 * Evaluate the battery state and change the battery flag accordingly.
	 * A fully charged battery should give out 4.2V.
	 * A completely drained battery will shut off at 3.2V.
	 * The output generally stays around 3.7V for most of the time, then slowly drops to 3.2V.
	 * At 3.2V, the battery's own circuitry will shut it off. Absolute minimum voltage is around 2.6V.
	 * Software will indicate a battery low state if the voltage drops below roughly 3.4V.
	 */

	enum_Yes_No_Selector batter_state_yes_no_buffer;

	float battery_readout = Battery_ADC_Readout();

	if(battery_readout < (float) 3.4){

		batter_state_yes_no_buffer = Yes;

	} else if(battery_readout >= (float) 3.4){

		batter_state_yes_no_buffer = No;

	}

	return batter_state_yes_no_buffer;
}
