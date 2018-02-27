/**
 * This is template for main module created by New Kinetis SDK 2.x Project Wizard. Enjoy!
 **/

#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "fsl_gpio.h"
#include "fsl_uart.h"
#include "fsl_port.h"
#include "fsl_common.h"
#include "fsl_i2c.h"
#include "fsl_smc.h"
#include "fsl_lptmr.h"
#include "fsl_dspi.h"

#include <stdio.h>
#include <stdlib.h>

#include "acc_func.h"
#include "i2c_func.h"
#include "adc_func.h"
#include "fsl_rtc.h"

void delay(uint32_t del) {
	for (; del > 1; del--) {
		__asm("nop");
	}
}
void initTimer() {

	lptmr_config_t lptmr_config;
	LPTMR_GetDefaultConfig(&lptmr_config);
	lptmr_config.bypassPrescaler = true;
	lptmr_config.value = kLPTMR_Prescale_Glitch_0;
	lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
	EnableIRQ(LPTMR0_IRQn);
	LPTMR_Init(LPTMR0, &lptmr_config);
	LPTMR_SetTimerPeriod(LPTMR0, 5000);  // 3000 for 20hz data rat
}

void LPTMR0_IRQHandler() {

	LPTMR0 -> CSR |= LPTMR_CSR_TCF_MASK;		// Clear the interrupt flag
	while ( LPTMR0 -> CSR & LPTMR_CSR_TCF_MASK  ) {

	}
	GPIO_PortToggle(GPIOB, 1<<21u); //light blue LED
}

int main(void) {
  /* Init board hardware. */
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_InitDebugConsole();
  initI2C();
  initAdc();
  configure_acc();
  acc_init();
  initTimer();
  EnableIRQ(PORTC_IRQn);
  LPTMR_EnableInterrupts(LPTMR0, LPTMR_CSR_TIE_MASK);		//Sets Timer Interrupt Enable bit to 1
  LPTMR_StartTimer(LPTMR0);

  static const gpio_pin_config_t LED_configOutput = { kGPIO_DigitalOutput, /* use as output pin */
  	1, /* initial value */
  	};

  GPIO_PinInit(GPIOB, 21u, &LED_configOutput);
  GPIO_PinInit(GPIOB, 22u, &LED_configOutput);

  while (true) {
/*
	  GPIO_ClearPinsOutput(GPIOB, 1<<22u); //light red LED
	  delay(10000000);
	  GPIO_SetPinsOutput(GPIOB, 1<<22u); //turn off red LED
	  delay(10000000);
*/

  }

}

void PORTC_IRQHandler() {

	PORTC -> PCR[13] |= 0x01000000;

	DisableIRQ(LPTMR0_IRQn);
	LPTMR0 -> CSR |= LPTMR_CSR_TCF_MASK;		// Clear the interrupt flag
	while ( LPTMR0 -> CSR & LPTMR_CSR_TCF_MASK  ) {

	}

	GPIO_PortToggle(GPIOB, 1<<22u); //light red LED

	EnableIRQ(LPTMR0_IRQn);

}
