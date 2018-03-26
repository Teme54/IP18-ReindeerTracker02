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
#include "nbiot_func.h"
#include "timing.h"

#define RESPONSE_TIMEOUT_NORMAL_VALUE 2000

lptmr_config_t lptmr_config;
smc_power_mode_vlls_config_t smc_power_mode_vlls_config;
uart_config_t uart_config;

volatile uint8_t wake = 0;
volatile uint8_t UART3_strReady = 0;
volatile uint16_t UART3_bufPtr = 0;

char UART3_recBuf[1000]; 	//buffer for receiving NB IoT module data

static char PC_recBuf[500];	//buffer for receiving from PC terminal
volatile uint16_t PC_bufPtr = 0;
volatile uint8_t PC_strReady = 0;

static char GPS_recBuf[500];	//buffer for receiving from PC terminal
volatile uint16_t GPS_bufPtr = 0;
volatile uint8_t GPS_strReady = 0;
uint8_t streamGps = 0;

char parsedLat[15];
char parsedLon[15];

//char testLon[11] = ("00833.91565");
//char testLat[11] = ("4717.11364");

char testLon[11] = ("33.91565");
char testLat[11] = ("17.11364");

extern char ubx_cfg_prt[];
extern char PMC_set[];
extern const char ubx_ack[];

volatile uint32_t moduleResponseTimeout = RESPONSE_TIMEOUT_NORMAL_VALUE; //timeout variable for waiting all data from module

uint32_t ms_ticks; //millisecond ticks value for the delay_ms function


/*
 * Init dead reindeer timer. LPTIMER interrupt will wake up MCU after a certain time, IF accelerometer interrupt
 * has not waked it earlier (and reset the timer)
 */
void initTimer()
{

	LPTMR_GetDefaultConfig(&lptmr_config);
	lptmr_config.bypassPrescaler = true;
	lptmr_config.value = kLPTMR_Prescale_Glitch_0;
	lptmr_config.prescalerClockSource = kLPTMR_PrescalerClock_1;
	//EnableIRQ(LPTMR0_IRQn);
	LPTMR_Init(LPTMR0, &lptmr_config);
	LPTMR_SetTimerPeriod(LPTMR0, 2000);  // 3000 for 20hz data rat
}

/*
 *
 * Init all needed UART buses. UART3 for NB-IoT, UART0 for PC, UART2 for GPS
 */

void initUART()
{

	uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ; //get MCU clock frequency for setting correct baudrate

	UART_GetDefaultConfig(&uart_config);
	uart_config.baudRate_Bps = 9600;
	uart_config.enableTx = true;
	uart_config.enableRx = true;
	UART_Init(UART3, &uart_config, uartClkSrcFreq); //Init UART3 for NBiot

	UART_Init(UART2, &uart_config, uartClkSrcFreq); //UART2 for GPS with same settings!

	UART_EnableInterrupts(UART3, kUART_RxDataRegFullInterruptEnable); //enable UART3 receive interrupt to receive data from NBiot
	EnableIRQ(UART3_RX_TX_IRQn);

	UART_EnableInterrupts(UART2, kUART_RxDataRegFullInterruptEnable); //enable UART3 receive interrupt to receive data from GPS
	EnableIRQ(UART2_RX_TX_IRQn);

	UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable); //enable UART0 receive interrupt to receive data from PC
	EnableIRQ(UART0_RX_TX_IRQn);



}

/*
 * Send data to NBiot with UART3
 * String to be sent is pointed by *data
 *
 */

void UART3_send(char *data)
{

	char c = *data++; //assign c a character from the string and post-increment string pointer

	while (c)
	{ //loop until c is zero which means string has ended and no more chars has to be sent

		while (!((UART3->S1) & 0x80))
		{
		} //wait until UART3 Transmission Complete flag rises, so we can send new char
		UART3->D = c; //write new character to transmit buffer
		c = *data++; //assign next character to c and post-increment string pointer
	}
}

uint8_t UART3_receive()
{

	if (UART3_strReady)
	{
		//printf("Received raw buffer: %s\r\n", UART3_recBuf);
		UART3_strReady = 0;
		memset(UART3_recBuf, 0, strlen(UART3_recBuf));

		return 1;
	}
	return 0;
}


/*
 * Send byte array to GPS module. This works different than other send functions
 * This will not handle input data as string, but as a byte array, so it will not stop sending to 0x00 character
 * It will send len.number of bytes instead
 *
 */
void UART2_send(char *data, uint8_t len)
{

	char c = *data++; //assign c a character from the string and post-increment string pointer

	for(;len>0;len--)
	{ //loop until c is zero which means string has ended and no more chars has to be sent

		while (!((UART2->S1) & 0x80))
		{
		} //wait until UART3 Transmission Complete flag rises, so we can send new char
		UART2->D = c; //write new character to transmit buffer
		c = *data++; //assign next character to c and post-increment string pointer
	}
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

int main(void)
{

	uint8_t res;

	PMC->REGSC |= 0x08;	//acknowledge wake up to voltage regulator module, this is needed with LLWU wake up
	EnableIRQ(LLWU_IRQn);//enable LLWU interrupts. if we wake up from VLLS mode, it means that next MCU
						 //will jump to the LLWU interrupt vector

	struct reindeerData_t reindeerData; //create struct for our reindeer data that will be sent
	char udpMessage[350];

	BOARD_InitPins();	//init all physical pins
	//BOARD_BootClockRUN();  //by uncommenting this we can use FRDM 50Mhz external clock, but will not work with modified board
	BOARD_InitDebugConsole();

	SysTick_Config(BOARD_DEBUG_UART_CLK_FREQ / 1000); //setup SysTick timer for 1ms interval for delay functions(see timing.h)

	initI2C();
	initAdc();
	initUART();
	configure_acc();
	acc_init();
	initTimer();

	SMC_SetPowerModeProtection(SMC, kSMC_AllowPowerModeVlls);
	smc_power_mode_vlls_config.subMode = kSMC_StopSub1; /*!< Stop submode 1, for VLLS1/LLS1. */

	LLWU->ME |= 0x01; 		// enable LLWU wakeup source from LPTMR module
	LLWU->PE3 |= 0x20; // enable LLWU wakeup source from accelerometer interrupt pin
	LLWU->FILT1 |= 0x4A;	// set pin wakeup from rising edge

	//EnableIRQ(PORTC_IRQn);

	LPTMR_EnableInterrupts(LPTMR0, LPTMR_CSR_TIE_MASK);	//Sets Timer Interrupt Enable bit to 1
	LPTMR_StartTimer(LPTMR0);

	static const gpio_pin_config_t LED_configOutput =
	{ kGPIO_DigitalOutput, /* use as output pin */
	1, /* initial value */
	};

	GPIO_PinInit(GPIOB, 21u, &LED_configOutput);	//red led as output
	GPIO_PinInit(GPIOB, 22u, &LED_configOutput);	//blue led as output

	/*
	 * set boost regulator enable pin as output. This pin will control the power to RF modules
	 */
	GPIO_PinInit(GPIOB, 11u, &LED_configOutput);

	GPIO_SetPinsOutput(GPIOB, 1 << 11u); //Power on RF modules

	//fletcher8(PMC_set, 14);
	//fletcher8(ubx_cfg_prt, 7);
	//ubx_send(ubx_cfg_prt);

	printf(
			"Reindeer IoT has started\r\nCommand \"iot\" to start executing reindeer track cycle\r\n"
			"Command \"gpsinfo=1\" or \"gpsinfo=0\" to switch GPS data on/off\r\n");
	printf("Or enter normal AT commands here for SARA-N2\r\nModules powered on and booting now!\r\n");

	/*
	 * Copy all reindeer variables to struct before starting network operations
	 */

	strcpy(reindeerData.serialNum, "11111");
	strcpy(reindeerData.latitude, testLat);
	strcpy(reindeerData.longitude, testLon);
	strcpy(reindeerData.dead, "true");
	reindeerData.batteryLevel = 45;


	/*
	 * Assemble data to json format and then to POST message
	 */

	assemblePacket(&reindeerData, udpMessage);

	while (1)
	{
		//int16_t acc_val = read_acc_axis(0);
		//printf("Accelereometer %d\r\n",acc_val);

		/*
		 * Check if a string has arrived from PC (with CR line end)
		 */
		if (PC_strReady)
		{

			if (strstr(PC_recBuf, "iot") != NULL)
			{
				printf("Starting Reindeer IoT cycle\r\n");
				break;
			}
			else if(strstr(PC_recBuf, "gpsinfo=1") != NULL)
			{
				streamGps = 1;
			}
			else if(strstr(PC_recBuf, "gpsinfo=0") != NULL)
			{
				streamGps = 0;
			}
			else if(strstr(PC_recBuf, "\xb5\x62") != NULL) //if input is UBX command!
			{
				printf("send to gps\r\n");
				uint8_t ubxMsgLen = calcUbxCrc(PC_recBuf+2); //Calculate UBX checksum and add it to the message
				UART2_send(PC_recBuf, ubxMsgLen); //Send UBX message to module
			}
			else
			{

				UART3_send(PC_recBuf);

			}
			memset(PC_recBuf, 0, strlen(PC_recBuf));
				PC_strReady = 0;
				PC_bufPtr = 0;
		}
		if (UART3_strReady)
		{
			moduleResponseTimeout = millis()+RESPONSE_TIMEOUT_NORMAL_VALUE; //reset timeout to initial value

			while (millis() < moduleResponseTimeout)

			{

				if(breakIfAtOk())
				{
					break;
				}

			}

			//now the timeout has expired since last character had arrived, so we can process data

			printf(UART3_recBuf);printf("\r\n");
			memset(UART3_recBuf, 0, 1000);
			UART3_bufPtr = 0;
			UART3_strReady = 0;
		}

		/*
		 * If GPS string is ready and GPS data streaming is enabled, enter here to process GPS data buffer
		 */
		if(GPS_strReady && streamGps)
		{

			printf(GPS_recBuf);printf("\r\n"); //First print out whole buffer

			char* ubxResponseStartPtr = strstr(GPS_recBuf,"\xb5\x62"); //Find pointer to UBX header. If there is no UBX response, pointer
																	 	 //will be NULL


			if( ubxResponseStartPtr != NULL) //If pointer is not null, it means UBX response header is found
			{
				printf("Found UBX response\r\n");

				uint8_t responseLength = *(ubxResponseStartPtr+4); //Find out UBX response length, it is always the 5th byte
																	//from beginning of the packet. * means dereferencing pointer
																//dereferencing means "accessing the value where pointer points"

				printf("UBX response length: %02x\r\n", responseLength);

				printUbxResponseHex(ubxResponseStartPtr,responseLength+6+2); //Print UBX response message. Function wants to know
																		//how many chars to print. We must add 6+2 to print header and crc too
			}


			memset(GPS_recBuf, 0, 500);
			GPS_bufPtr = 0;
			GPS_strReady = 0;
		}

	}




	res = AT_send(AT_NRB, "", "+UFOTAS");
	if (res == 0)
	{
		printf("rebooted\r\n");
	}
	else if (res == 1)
	{
		printf("error\r\n");
	}
	else if (res == 2)
	{
		printf("timeout error\r\n");
	}

	res = NB_setPin();

	if (res == 0)
	{
		printf("ack\r\n");
	}
	else if (res == 1)
	{
		printf("error\r\n");
	}
	// delay_ms(20200020);
	NB_connectStatus();

	//parseData(testLat, testLon);

	while (true)
	{

		printf("Roger\r\n");
		delay_ms(1000);

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

void LLWU_IRQHandler()
{

	if ( LLWU->F3 & 0x01)
	{	// 1 = LPTMR interrupt, 2 = Accel interrupt, 0 = No interrupts
		wake = 1;
		CLOCK_EnableClock(kCLOCK_Lptmr0);
		LPTMR0->CSR |= LPTMR_CSR_TCF_MASK;
	}

	else if ( LLWU->F2 & 0x04)
	{
		wake = 2;
		LLWU->F2 |= 0x04;
	}

	LLWU->F2 = 0x04;
}

void UART3_RX_TX_IRQHandler()
{

	UART_ClearStatusFlags(UART3, kUART_RxDataRegFullFlag);
	GPIO_PortToggle(GPIOB, 1 << 22u); //toggle RED led to indicate data arrived from NB Iiootee module

	uint8_t uartData = UART3->D;
	if (uartData != 0)
	{

		UART3_recBuf[UART3_bufPtr] = uartData;
		UART3_bufPtr++;

		if (uartData == 0x0d)
		{
			UART3_strReady = 1;
			//UART3_bufPtr = 0;
		}

	}

}

void UART2_RX_TX_IRQHandler()
{

	UART_ClearStatusFlags(UART2, kUART_RxDataRegFullFlag);
	GPIO_PortToggle(GPIOB, 1 << 22u); //toggle RED led to indicate data arrived from GPS module

	uint8_t uartData = UART2->D;

	/*
	 * Here we use different method for collecting GPS data. because there can be other data than characters,
	 * like 0x00 in UBX messages, normal string functions would fail (mistaken null terminator)
	 * so we must collect every byte from the gps module
	 * so fill buffer to almost full with GPS data, then put GPS_strReady high and stop filling.
	 * Start filling again when data has been read and GPS_strReady is low.
	 *
	 */
	if(GPS_strReady == 0)
	{
		GPS_recBuf[GPS_bufPtr] = uartData; //put new byte to buffer
		GPS_bufPtr++;
	}


	/*
	 * When buffer is almost full, put strReady high and stop filling it
	 */
	if(GPS_bufPtr > 400)
	{
		GPS_strReady = 1;
	}

}

void UART0_RX_TX_IRQHandler()
{

	UART_ClearStatusFlags(UART0, kUART_RxDataRegFullFlag);
	GPIO_PortToggle(GPIOB, 1 << 21u); //toggle BLUE led to indicate data arrived from computer

	uint8_t uartData = UART0->D;


		PC_recBuf[PC_bufPtr] = uartData;
		PC_bufPtr++;

		if (uartData == 0x0d)
		{
			PC_strReady = 1;

		}


}
