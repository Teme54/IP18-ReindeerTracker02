/*
 * gps_func.c
 *
 *  Created on: Mar 15, 2018
 *      Author: nks
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gps_func.h"
#include "at_func.h"


void printUbxResponseHex(char* data, uint8_t dataLength)
{

	for(uint8_t n = 0;n<dataLength;n++)
	{
		printf("%02x ", (uint8_t)(*(data+n)));
	}

}

uint8_t calcUbxCrc(char *data)
{

	uint8_t ck_a = 0, ck_b = 0, n = 0;
	while(data[n] != 0x0d)
	{

		n++;
	}

	uint8_t dataLength = n;

	for(n=0; n<dataLength; n++)
	{
		ck_a = ck_a + data[n];
		ck_b = ck_b + ck_a;
	}

	data[n] = ck_a;
	data[n+1] = ck_b;
	data[n+2] = 0;

	for(n = 0;n<(dataLength+2);n++)
	{
		printf("%02x ", (uint8_t)data[n]);
	}

	return dataLength+4;
}

void getGPS() {

	char* token;
	const char s[2] = ",";
	uint8_t counter = 0;

	char GLL_ID[10];	// GLL Message ID $GPGLL
	char latitude[15];
	char northSouth[2];	// Indicates North / South, N = north, S = south
	char longitude[15];
	char eastWest[2];	// Indicates East / West, E = east, W = west
	char time[10];		// UTC Time
	char status[2];	// Status indicates data validity V = Data invalid or receiver warning, A = data valid
	char posMode[2];	// Positioning mode according to NMEA protocol
	char checkSum[10];

	char* GPS_dataPtrs[9] = { GLL_ID, latitude, northSouth, longitude, eastWest,
			time, status, posMode, checkSum };


	while (strstr(GPS_dataPtrs[6], "A") == NULL) {		// Loop until string contains A status, A = Data valid

		while (UART3_receive() == 0) { 	// Wait for a string from GPS module

		}


		if (strstr(UART3_recBuf, "$GNGLL") != NULL) {		// Check if received message is GPGLL message

			token = strtok(UART3_recBuf, s);				// Strtok splices the string to different variables, using "," separator

			while (token != NULL) {

				GPS_dataPtrs[counter] = token;				// Save the splices contents to different variables
				token = strtok(NULL, s);
				counter++;

			}

			if (strstr(GPS_dataPtrs[6], "V") != NULL) {
				printf("Data invalid, waiting for valid data\r\n");
			}

			else {

				for (uint8_t cnr = 0; cnr <= 8; cnr++) {
					printf("%s\r\n", GPS_dataPtrs[cnr]);
				}
			}

			memset(UART3_recBuf, 0, strlen(UART3_recBuf));

		}

		UART3_strReady = 0;
	}
	parseData(GPS_dataPtrs[1], GPS_dataPtrs[3]);
}

void parseData(char* latStr, char* lonStr) {

	uint8_t d_ptr = 0;

	const char s = '.';

	char* delPtr;

	latStr[4] = latStr[3];
	latStr[3] = latStr[2];
	latStr[2] = '.';

	lonStr[5] = lonStr[4];
	lonStr[4] = lonStr[3];
	lonStr[3] = '.';

	while ( latStr[d_ptr] == '0' ) {		// Skip all zeroes from beginning of string
		d_ptr++;
	}

	strcpy(parsedLat, latStr+d_ptr);		// Copy string without zeroes to new string

	d_ptr = 0;

	delPtr = strchr(parsedLat, s);

	uint32_t latMinutes = atol(delPtr+1);
	latMinutes = latMinutes/6;

	sprintf(delPtr+1, "%ld\r\n", latMinutes);

	while ( lonStr[d_ptr] == '0' ) {
		d_ptr++;
	}

	strcpy(parsedLon, lonStr+d_ptr);

	delPtr = strchr(parsedLon, s);

	uint32_t lonMinutes = atol(delPtr+1);
	lonMinutes = lonMinutes/6;

	sprintf(delPtr+1, "%ld\r\n", lonMinutes);

	printf("Parsed latitude: %s", parsedLon);
	printf("Parsed longitude: %s", parsedLat);
}
