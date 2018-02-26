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

/*!
 * @brief Application entry point.
 */
int main(void) {
  /* Init board hardware. */
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_InitDebugConsole();
  initI2C();
  initAdc();
  configure_acc();
  acc_init();
  EnableIRQ(PORTC_IRQn);


  static const gpio_pin_config_t LED_configOutput = { kGPIO_DigitalOutput, /* use as output pin */
  	1, /* initial value */
  	};

  GPIO_PinInit(GPIOB, 21u, &LED_configOutput);
  GPIO_PinInit(GPIOB, 22u, &LED_configOutput);

  while (true) {
	  GPIO_ClearPinsOutput(GPIOB, 1<<22u); //light red LED
	  delay(1000000);
	  GPIO_SetPinsOutput(GPIOB, 1<<22u); //turn off red LED
	  delay(1000000);
  }

  /* Add your code here */

}

void PORTC_IRQHandler() {

	PORTC -> PCR[13] |= 0x01000000;
	GPIO_PortToggle(GPIOB, 1<<21u); //light red LED
}
