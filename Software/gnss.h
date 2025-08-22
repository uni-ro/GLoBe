/*
 * gnss.h
 *
 *  Created on: Nov 4, 2024
 *      Author: Raph
 */

#ifndef INC_GNSS_H_
#define INC_GNSS_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "string.h"
#include "stm32h7xx_hal.h"

typedef enum {INVALID, NONE, GPS, GLONASS, GALILEO, BEIDOU} Constellation;
typedef enum {INV, DTM, GAQ, GBQ, GBS, GGA, GLL, GLQ, GNQ, GNS, GPQ, GRS, GSA, GST, GSV, RLM, RMC, TXT, VLW, VTG, ZDA} Sentences;

typedef struct GNSSData {
	Constellation constellation;
	double_t latitude;
	double_t longitude;
	uint8_t satellites;
	uint32_t GPStime;
	uint32_t date;
	char quality[32];

	uint8_t diffTime;
	float_t diffID;

	char ID[8];

	float_t PDOP;
	float_t HDOP;
	float_t VDOP;

	float_t altitude;
	float_t geoidSep;

	float_t angle;
	float_t speed;
	float_t magnetic;
} GNSSData;

GNSSData* createGNSSData();
uint16_t numberOfTokens(const char* data, const uint16_t length, const char token);
char * strnext(char ** input, const char * delim);
uint16_t numTokens(const char* string, const char * token);
uint8_t convertToDegree(const char* data, const char direction, double_t* dest);

char* getQuality(const char data);

uint8_t obtainUARTData(UART_HandleTypeDef *uart, uint8_t *buf, uint16_t size);
Sentences getSentenceType(const char * header)

char ** splitString(const char * string, const char * delim, uint16_t * arr_size);

GNSSData* parseNMEAData(const char* data);
char * getOpMode(char type);
char * getID(char type);
uint8_t captureGSAData(char ** lineArr, uint16_t length, GNSSData * gnssData);
uint8_t captureVTGData(char ** lineArr, uint16_t length, GNSSData * gnssData);
uint16_t captureGGAData(char ** lineArr, uint16_t length, GNSSData * gnssData);
uint8_t captureRMCData(char ** lineArr, uint16_t length, GNSSData * gnssData);
uint8_t captureGLLData(char ** lineArr, uint16_t length, GNSSData * gnssData);
// Does NMEA checksum to make sure data is correctly received
int8_t nmeaChecksum(const char *data);
int8_t verifyFormat(const char *data);
// Verifies data separated by NMEA messages and removes invalid or incorrect data
Constellation verifyData(const char* data, const uint16_t length);
Constellation convertConstellation(const char *header);
// Check if the data is valid and returns the constellation type
Constellation verifyValidData(const char *data);
#endif /* INC_GNSS_H_ */
