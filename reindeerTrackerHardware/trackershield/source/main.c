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
#include "at_func.h"

#include <stdio.h>
#include <stdlib.h>

#include "acc_func.h"
#include "i2c_func.h"
#include "adc_func.h"
#include "fsl_rtc.h"
#include "gps_func.h"
#include "ubx_func.h"

lptmr_config_t lptmr_config;
smc_power_mode_vlls_config_t smc_power_mode_vlls_config;
uart_config_t uart_config;

volatile uint8_t wake = 0;
volatile uint8_t UART3_strReady = 0;
volatile uint16_t UART3_bufPtr = 0;

char UART3_recBuf[1000];
char UART3_strBuf[1000];



char parsedLat[15];
char parsedLon[15];


char testLon[11] = ("00833.91565");
char testLat[11] = ("4717.11364");

extern char ubx_cfg_prt[];
extern char PMC_set[];
extern const char ubx_ack[];

void delay(uint32_t del) {
	for (; del > 1; del--) {
		__asm("nop");
	}
}


void initTimer() {

	LPTMR_GetDefaultConfig(&lptmr_config);
	lptmr_config.bypassPrescaler = true;
	lptmr_config.value = kLPTMR_Prescale_Glitch_0;
	lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
	//EnableIRQ(LPTMR0_IRQn);
	LPTMR_Init(LPTMR0, &lptmr_config);
	LPTMR_SetTimerPeriod(LPTMR0, 2000);  // 3000 for 20hz data rat
}

void initUART() {

	uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
	UART_GetDefaultConfig(&uart_config);
	uart_config.baudRate_Bps = 9600;
	uart_config.enableTx = true;
	uart_config.enableRx = true;
	UART_Init(UART3, &uart_config, uartClkSrcFreq);
	UART_EnableInterrupts(UART3,kUART_RxDataRegFullInterruptEnable);
	EnableIRQ(UART3_RX_TX_IRQn);

}

void UART3_send(char *data) {

	char c = *data++;
	while (c) {
		while(!((UART3->S1) & 0x80)) {}
		UART3->D = c;

		c = *data++;
	}
}

uint8_t UART3_receive() {

	if (UART3_strReady) {

		//printf("Received raw buffer: %s\r\n", UART3_recBuf);

		UART3_strReady = 0;

		memset(UART3_recBuf, 0, strlen(UART3_recBuf));

		return 1;
	}
	return 0;
}

/*
void LPTMR0_IRQHandler() {

	LPTMR0 -> CSR |= LPTMR_CSR_TCF_MASK;		// Clear the interrupt flag
	while ( LPTMR0 -> CSR & LPTMR_CSR_TCF_MASK  ) {

	}

	PMC ->REGSC |= 0x08;

	GPIO_PortToggle(GPIOB, 1<<21u); //light blue LED
}
*/

int main(void) {

	 uint8_t res;
	//uint8_t temp = LLWU -> F3;
	PMC ->REGSC |= 0x08;
	EnableIRQ(LLWU_IRQn);

	/* Init board hardware. */
	BOARD_InitPins();
	//BOARD_BootClockRUN();
	BOARD_InitDebugConsole();

  initI2C();
  initAdc();
  initUART();
  configure_acc();
  acc_init();
  initTimer();


 SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
  smc_power_mode_vlls_config.subMode = kSMC_StopSub1; 			/*!< Stop submode 1, for VLLS1/LLS1. */

  LLWU -> ME |= 0x01; 											// Enable wakeup module for LPTMR
  LLWU -> PE3 |= 0x20;
  LLWU -> FILT1 |= 0x4A;
  //LLWU -> RST |= 0x01;

  //EnableIRQ(PORTC_IRQn);

  LPTMR_EnableInterrupts(LPTMR0, LPTMR_CSR_TIE_MASK);		//Sets Timer Interrupt Enable bit to 1
  LPTMR_StartTimer(LPTMR0);


  static const gpio_pin_config_t LED_configOutput = { kGPIO_DigitalOutput, /* use as output pin */
  	1, /* initial value */
  	};

  GPIO_PinInit(GPIOB, 21u, &LED_configOutput);
  GPIO_PinInit(GPIOB, 22u, &LED_configOutput);

  GPIO_ClearPinsOutput(GPIOB, 1<<22u); //light red to indicate interrupt LEDd
  delay(100000);
  GPIO_SetPinsOutput(GPIOB, 1<<22u); //light red to indicate interrupt LED



  //fletcher8(PMC_set, 14);
  //fletcher8(ubx_cfg_prt, 7);
  ubx_send(ubx_cfg_prt);


  //printf("waked IN MAIN HAHA %d\r\n", wake);


	res = AT_send(AT_NRB, "", "+UFOTAS");
			if (res == 0) {
				printf("rebooted\r\n");
			} else if (res == 1) {
				printf("error\r\n");
			}
			else if (res == 2) {
							printf("timeout error\r\n");
						}


 	  res =  NB_setPin();

 	  if(res == 0){
 		  printf("ack\r\n");
 	  }
 	  else if(res == 1){
 		  printf("error\r\n");
 	  }
 	 // delay(20200020);
 	  NB_connectStatus();







  //parseData(testLat, testLon);


  while (true) {

	  printf("Roger\r\n");
	  delay(20020020);


	  /*
	  printf("waked lul %d\r\n", wake);


	  if ( wake == 1 ) {
		  wake = 0;
		  //getGPS();
		  printf("Got GPS\r\n");
	  }

	  LPTMR0 -> CNR = 0;				// Prints the timer value, write anything on counter before reading it

	  //printf("Timer value: %ld \r\n", LPTMR0 -> CNR);

	  SMC_PreEnterStopModes();

	  SMC_SetPowerModeVlls(SMC, &smc_power_mode_vlls_config);

	  SMC_PostExitStopModes();

*/
  }

}
/*
void PORTC_IRQHandler() {

	PORTC -> PCR[6] |= 0x01000000;

	while ( PORTC -> PCR[6] & 0x01000000 ) {

	}

	LPTMR_Deinit(LPTMR0);			// Deinitiate timer to reset timer counte
	LPTMR_Init(LPTMR0, &lptmr_config);
	LPTMR_SetTimerPeriod(LPTMR0, 5000);  // 3000 for 20hz data rat
	LPTMR_EnableInterrupts(LPTMR0, LPTMR_CSR_TIE_MASK);	//Sets Timer Interrupt Enable bit to 1
	LPTMR_StartTimer(LPTMR0);

}
*/

void LLWU_IRQHandler() {

	if ( LLWU -> F3 & 0x01 ) {	// 1 = LPTMR interrupt, 2 = Accel interrupt, 0 = No interrupts
		wake = 1;
		CLOCK_EnableClock(kCLOCK_Lptmr0);
		LPTMR0 -> CSR |= LPTMR_CSR_TCF_MASK;
	}

	else if ( LLWU -> F2 & 0x04) {
		wake = 2;
		LLWU -> F2 |= 0x04;
	}

	LLWU -> F2 = 0x04;
}

void UART3_RX_TX_IRQHandler() {

	UART_ClearStatusFlags(UART3,kUART_RxDataRegFullFlag);
	GPIO_PortToggle(GPIOB, 1 << 21u);

	uint8_t uartData = UART3 -> D;
	if(uartData != 0){
	UART3_recBuf[UART3_bufPtr] = uartData;
	UART3_bufPtr++;


	if ( uartData == 0x0A) {

		UART3_strReady = 1;

		//UART3_bufPtr = 0;

	}
}

}
