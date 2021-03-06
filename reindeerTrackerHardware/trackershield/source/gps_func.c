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



/*
 *
 * Print UBX response message as hex numbers
 * cannot print it normally by printf because it contains 0x00 as data
 */

void printUbxResponseHex(char* data, uint8_t dataLength)
{
	for (uint8_t n = 0; n < dataLength; n++)
	{
		printf("%02x ", (uint8_t) (*(data + n)));
	}
}

uint8_t calcUbxCrc(char *data)
{

	uint8_t ck_a = 0, ck_b = 0, n = 0;

	while (data[n] != 0x0d) //figure out length of input message by finding CR line end char
	{
		n++;
	}

	uint8_t dataLength = n;

	for (n = 0; n < dataLength; n++)//calculate checksum for dataLength bytes
	{
		ck_a = ck_a + data[n];
		ck_b = ck_b + ck_a;
	}

	data[n] = ck_a; //add checksum to end of data string
	data[n + 1] = ck_b;
	data[n + 2] = 0;	//add a zero to terminate string

	for (n = 0; n < (dataLength + 2); n++) //debug print our complete message
	{
		printf("%02x ", (uint8_t) data[n]);
	}

	return dataLength + 4;
}

uint8_t getGPS()
{

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

	char* GPS_dataPtrs[9] =
	{ GLL_ID, latitude, northSouth, longitude, eastWest, time, status, posMode,
			checkSum };

	char* gllStart = strstr(GPS_recBuf, "NGLL"); //get starting pointer of GLL string
	char* gsvStart = strstr(GPS_recBuf, "PGSV"); //get start of GSV string(sats in view)

	if (gllStart != NULL) //check if GLL string is found
	{

		char* gllEnd = strstr(gllStart, "\r\n"); //get end of GLL string

		if (gllEnd != NULL)

		{
			*(gllEnd + 2) = 0; //write 0 to end of GLL string so string functions will stop correctly
			printf("GLL string %s\r\n", gllStart);
		}

		uint8_t gllLength = strlen(gllStart);
		char* gllStringPtr = gllStart;

		/*
		 * Here own function is used instead of strtok to splice the string
		 *
		 */
		while (*gllStringPtr)
		{

			if (*gllStringPtr == ',') //if comma is found
			{
				*gllStringPtr = 0; //put 0 to comma's place
				GPS_dataPtrs[counter] = gllStart; //save this string token to variable
				counter++;
				gllStart = gllStringPtr + 1; //move gllStart so it points to the next char after the comma

			}

			gllStringPtr++;

		}

		if (strstr(GPS_dataPtrs[6], "A") != NULL)
		{
			printf("Got GPS\r\n");
			parseData(GPS_dataPtrs[1], GPS_dataPtrs[3]);
			return 1;
		}

		else if (strstr(GPS_dataPtrs[6], "V") != NULL)
		{
			printf("Data invalid, waiting for valid data\r\n");
		}

		for (uint8_t cnr = 0; cnr < 7; cnr++)
		{
			printf("%s\r\n", GPS_dataPtrs[cnr]);
		}

	}

	if (gsvStart != NULL) //if GSV string is found, print it
	{
		char* gsvEnd = strstr(gsvStart, "\r\n");

		if (gsvEnd != NULL)
		{

			*(gsvEnd + 2) = 0;
			printf("GSV string %s\r\n", gsvStart);

		}
	}

	return 0;
	//}
}

void parseData(char* latStr, char* lonStr)
{

	uint8_t d_ptr = 0;

	const char s = '.';

	char* delPtr;

	/*
	 * swap comma from ddmm.mmm to dd.mmmmm
	 */

	latStr[4] = latStr[3];
	latStr[3] = latStr[2];
	latStr[2] = '.';

	lonStr[5] = lonStr[4];
	lonStr[4] = lonStr[3];
	lonStr[3] = '.';

	printf("latstr %s lonstr %s\r\n",latStr,lonStr);

	while (latStr[d_ptr] == '0')
	{		// Skip all zeroes from beginning of string
		d_ptr++;
	}

	strcpy(parsedLat, latStr + d_ptr);// Copy string without zeroes to new string

	d_ptr = 0;

	delPtr = strchr(parsedLat, s);
	delPtr++;
	while(*delPtr == '0'){	delPtr++;} //LOOP2: If there is zeroes in minutes part, skip them

	uint32_t latMinutes = atol(delPtr);
	latMinutes = latMinutes / 6; //Convert minutes to decimal degrees

	sprintf(delPtr, "%ld\r\n", latMinutes); //here because of LOOP2 delPtr points after the original zeroes(if there were any)
											//and decimal problem is fixed

	printf("latmin: %ld\r\n",latMinutes);

	while (lonStr[d_ptr] == '0')
	{
		d_ptr++;
	}

	strcpy(parsedLon, lonStr + d_ptr);

	delPtr = strchr(parsedLon, s);
	delPtr++;
	while(*delPtr == '0'){	delPtr++;} //LOOP3: If there is zeroes in minutes part, skip them

	uint32_t lonMinutes = atol(delPtr);
	lonMinutes = lonMinutes / 6;

	sprintf(delPtr, "%ld\r\n", lonMinutes);//here because of LOOP3 delPtr points after the original zeroes(if there were any)
											//and decimal problem is fixed

	//printf("Parsed latitude: %s", parsedLon);
	//printf("Parsed longitude: %s", parsedLat);
}
