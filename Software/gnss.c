#include "gnss.h"

/* Helper functions */
GNSSData* createGNSSData() {
	GNSSData *data = (GNSSData*) malloc(sizeof(GNSSData));
	data->constellation = INVALID;
	data->latitude = 0.0;
	data->longitude = 0.0;
	data->satellites = 0;
	data->GPStime = 0;
	data->date = 0;
	data->quality[0] = '\0';
	data->diffTime = 0;
	data->diffID = 0;
	data->ID[0] = '\0';
	data->PDOP = 0;
	data->HDOP = 0;
	data->VDOP = 0;
	data->altitude = 0.0;
	data->geoidSep = 0.0;
	data->angle = 0.0;
	data->speed = 0.0;
	data->magnetic = 0.0;
	return data;
}

uint16_t numberOfTokens(const char *data, const uint16_t length, const char token) {
	uint16_t i;
	uint16_t t = 0;
	for (i = 0; i < length; i++) {
		if (data[i] == token) {
			t++;
		}
	}
	return t + 1;
}

/**
 * Similar to `strsep` - updates the input string to point past the
 * delimiter string and sets the delimiter part to \0. Uses the entire
 * delim instead of only one of the characters from it like `strsep`.
 * 
 * @param input The input string to update past the delim.
 * @param delim The delimiter to stop at and to replace.
 * 
 * @returns The part of the string that was skipped.
 * 
 * @note The function will point past a string if the final character
 * 		 is not a null byte (\0).
 */
char * strnext(char ** input, const char * delim)
{
	char * string = *input;
	char * found;
	uint8_t delimLength = strlen(delim);

	found = strstr(*input, delim);

	if (found != NULL)
	{
		*found = '\0';
		*input = found + delimLength;
	}

	return string;
}

/**
 * Searches the given string for all instances of the given token and returns
 * the number of occurrences.
 * 
 * @param string The string to search for the tokens
 * @param token The token to find within the string
 * 
 * @returns The number of times `token` was found in `string`
 */
uint16_t numTokens(const char *string, const char * token)
{
	uint16_t nTokens = 0;
	uint16_t i = 0;
	uint8_t tokenLength = strlen(token);

	char * substr = (char *) calloc(tokenLength + 1, sizeof(char));

	/* Scan the string for instances of the token, ending with the final token size */
	for (i = 0; i < strlen(string) - tokenLength + 1; i++)
	{
		substr = strncpy(substr, &(string[i]), tokenLength);

		if (strcmp(token, substr) == 0)
		{
			nTokens++;
		}
	}

	free(substr);

	return nTokens;
}

uint8_t convertToDegree(const char *data, const char direction, double_t *dest) {
	char *error_ptr;
	uint8_t whole = (uint8_t) (strtol(data, &error_ptr, 10) / 100); //Safely convert to whole number, atoi is unsafe
	if (*error_ptr == '.') {
		double_t decimal = strtod(data, &error_ptr);
		if (*error_ptr == '\0') {
			// Success
			decimal = (double_t) (((double_t) (decimal
					- (double_t) (whole * 100))) / 60.0) + (double_t) (whole);
			if (direction == 'S' || direction == 'W') {
				decimal *= -1;
			}
			*dest = decimal;
			return 0;
		}
	}
	printf("Could not convert to degree: %s%c\r\n", data, direction);
	return -1;
}

char* getQuality(const char data) {
	if (data == '0') {
		return "No fix";
	} else if (data == '1') {
		return "Autonomous";
	} else if (data == '2') {
		return "Differential";
	} else if (data == '4') {
		return "RTK fixed";
	} else if (data == '5') {
		return "RTK Float";
	} else if (data == '6') {
		return "Estimated";
	}
	if (data == 'N') {
		return "No fix";
	} else if (data == 'A') {
		return "Autonomous";
	} else if (data == 'D') {
		return "Differential";
	} else if (data == 'R') {
		return "RTK fixed";
	} else if (data == 'F') {
		return "RTK Float";
	} else if (data == 'E') {
		return "Estimated";
	}
	return '\0';
}

/* Reading data */

void scanI2C(I2C_HandleTypeDef *i2c) {
	uint8_t i = 0;

	/*-[ I2C Bus Scanning ]-*/
	for (i = 1; i < 128; i++) {
		if (HAL_I2C_IsDeviceReady(i2c, (uint16_t) (i << 1), 3, 5) == HAL_OK) {
			printf("0x%X\r\n", i);
		}
	}
	/*--[ Scanning Done ]--*/
}

uint8_t obtainI2CData(I2C_HandleTypeDef *i2c, uint8_t addr, uint8_t *buf,
		uint16_t size) {
	HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(i2c,
			(uint16_t) (addr << 1), 3, 5);
	uint8_t sizeU = 0;
	uint8_t sizeL = 0;
	uint16_t dataSize = 0;
	if (status == HAL_OK) {
		HAL_I2C_Mem_Read(i2c, (uint16_t) (addr << 1), 0xFD, 1, &sizeU,
				sizeof(sizeU), 5);
		HAL_I2C_Mem_Read(i2c, (uint16_t) (addr << 1), 0xFE, 1, &sizeL,
				sizeof(sizeL), 5);
		dataSize = (sizeU << 8) + sizeL;
		if (dataSize > 0) {
			/*printf("Data entering of size %d obtained from %d %d\r\n", dataSize,
			 sizeU, sizeL);*/
			//HAL_I2C_Mem_Read(&hi2c1, (uint16_t)(addr<<1), 0xFF, 1, pData, size, 1000);
			if (dataSize > size) {
				printf("Too much data: %d, buffer too small\r\n", dataSize);
				dataSize = size;
			}
			HAL_I2C_Mem_Read(i2c, (uint16_t) (addr << 1), 0xFF, 1, buf,
					dataSize, 1000);
			return 0;
		}
	} else {
		printf("Bad status: %d\r\n", status);
	}
	return -1;
}

uint8_t obtainUARTData(UART_HandleTypeDef *uart, uint8_t *buf, uint16_t size) {
	return 0;
}

/**
 * Splits the given string into an array of strings at each instance of delim.
 *
 * @param string The string that is going to be split.
 * @param delim The delimiter to split at, removing the given delimiter.
 * @param arr_size A pointer that will be populated with the resulting size of the array.
 *
 * @returns An array of strings split at the delimiter.
 *
 * @note This keeps the array of strings within the bounds of the original string.
 * ```
 * 		 For example, "This * string" becomes ["This ", " string"] at the same address.
 * 		 To clarify, "This * string" actually becomes "This \0 string"
 * 					  ^                                ^      ^
 * 		 where the '^' character indicates the position of each pointer.
 * ```
 * 		 Notice that initially
 * 		 there is 1 pointer as it is 1 string, but the second instance contains 2 pointers
 * 		 as this essentially becomes an array of pointers pointing to different locations
 * 		 within the same string.
 * 		 As such, to free the data that has been allocated, only both the return value and its
 * 		 first element must be freed. This is evident in the below example:
 *
 * ```
 * 		char ** arr = splitString("This * string", "*", 2);
 *
 * 		free(*arr);
 * 		free(arr);
 * ```
 *
 * @note This functionality is subject to change
 */
char ** splitString(const char * string, const char * delim, uint16_t * arr_size)
{
	uint16_t nParts = numTokens(string, delim) + 1; /* For n splits, there are n+1 sections */

	/* Create copy of input string for strsep to use */
	char * input = (char *) calloc(strlen(string) + 1, sizeof(char));
	input = strcpy(input, string);

	/* If the string ends with the delimiter, create only n sections
	 * For example, Foo\r with delimiter as '\r' returns an array of size 2.
	 * 		Element 0 is: Foo 
	 * 		Element 1 is: \0
	 * 		Hence, the array size should be corrected to 1 instead of 2.
	 */
	if (strcmp(input + strlen(input) - strlen(delim), delim) == 0)
	{
		nParts--;
	}

	char ** strings = (char **) calloc(nParts, sizeof(char *));

    uint16_t i = 0;

	for (i = 0; i < nParts; i++)
	{
		strings[i] = strnext(&input, delim);
	}

    *arr_size = nParts;


	return strings;
}

GNSSData* parseNMEAData(const char *data) {
	const char outer_delimiter[] = "\r\n";
	const char inner_delimiter[] = ",";


	uint16_t i = 0;

	uint16_t numOfLines = 0;
	uint16_t lineCount = 0;

	char ** lines = splitString(data, outer_delimiter, &numOfLines);

	GNSSData *gnssData = createGNSSData();
	uint8_t status = -1;



	// Temporary variables
	char *error_ptr;
	float_t tempFloat;
	uint32_t temp32;
	uint8_t temp8;
	uint16_t length;
	uint16_t tokenNum;


	char *line = lines[lineCount];
	char *lineStart = NULL;

	while (line != NULL) {
		lineStart = strrchr(line, '$');
		if (lineStart != NULL) { // Realign the last occurence of $ to be start of message
			line = lineStart;
		}
		length = strlen(line); // Length of string
		tokenNum = numberOfTokens(line, length, ','); // Number of tokens in line

		Constellation constellation = verifyData(line, length); // Check headers to make sure they start in NMEA format

		if (constellation != INVALID && tokenNum > 1) { // If data is valid and contains data (more than just the head

			//printf("%s\r\n", line);

			// Tokenise data
			char *lineArray[tokenNum];
			i = 0;
			lineArray[i] = strsep(&line, inner_delimiter);
			while (lineArray[i] != NULL && i < tokenNum) {
				i++; // Adds first because there is an extra token at the end?
				lineArray[i] = strsep(&line, inner_delimiter);
			}

			// Assign data
			gnssData->constellation = constellation;
			if (strncmp("GSA", lineArray[0] + 3, 3) == 0) {
				if (lineArray[2][0] != '1') {
					if (lineArray[1][0] == 'M') {
						strcpy(gnssData->quality, "Manual");
					} else if (lineArray[1][0] == 'A') {
						strcpy(gnssData->quality, "Auto");
					} else {
						strcpy(gnssData->quality, "Unknown");
					}
					tempFloat = strtof(lineArray[tokenNum - 4], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->PDOP = tempFloat;
					}
					tempFloat = strtof(lineArray[tokenNum - 3], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->HDOP = tempFloat;
					}
					tempFloat = strtof(lineArray[tokenNum - 2], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->VDOP = tempFloat;
					}
					if (lineArray[tokenNum - 1][0] == '1') {
						strcpy(gnssData->ID, "GPS");
					} else if (lineArray[tokenNum - 1][0] == '2') {
						strcpy(gnssData->ID, "GLONASS");
					} else if (lineArray[tokenNum - 1][0] == '3') {
						strcpy(gnssData->ID, "GALILEO");
					} else if (lineArray[tokenNum - 1][0] == '5') {
						strcpy(gnssData->ID, "BEIDOU");
					}
					status = 0;
				}
			} else if (strncmp("GSV", lineArray[0] + 3, 3) == 0) {
				printf("GSV: ");
				for (i = 0; i < tokenNum - 1; i++) {
					printf("%s,", lineArray[i]);
				}
				printf("%s\r\n", lineArray[i + 1]);
			} else if (strncmp("VTG", lineArray[0] + 3, 3) == 0) {
				if (lineArray[2][0] == 'T' && lineArray[4][0] == 'M'
						&& lineArray[6][0] == 'N' && lineArray[8][0] == 'K'
						&& lineArray[9][0] != '1') {
					tempFloat = strtof(lineArray[1], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->angle = tempFloat;
					}
					tempFloat = strtof(lineArray[3], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->magnetic = tempFloat;
					}
					tempFloat = strtof(lineArray[5], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->speed = tempFloat;
					}
					char *quality = getQuality(lineArray[9][0]);
					if (quality[0] != '\0') {
						strcpy(gnssData->quality, quality);
					}
					status = 0;
				}
			} else if (strncmp("GGA", lineArray[0] + 3, 3) == 0) {
				if (lineArray[10][0] == 'M' && lineArray[12][0] == 'M'
						&& lineArray[6][0] != '0') {
					temp32 = strtol(lineArray[1], &error_ptr, 10);
					if (*error_ptr == '.') {
						gnssData->GPStime = temp32;
						status = 0;
					}
					convertToDegree(lineArray[2], lineArray[3][0],
							&(gnssData->latitude));
					convertToDegree(lineArray[4], lineArray[5][0],
							&(gnssData->longitude));
					char *quality = getQuality(lineArray[6][0]);
					if (quality[0] != '\0') {
						strcpy(gnssData->quality, quality);
					}
					temp8 = strtol(lineArray[7], &error_ptr, 10);
					if (*error_ptr == '\0') {
						gnssData->satellites = temp8;
					}
					tempFloat = strtof(lineArray[8], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->HDOP = tempFloat;
					}
					tempFloat = strtof(lineArray[9], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->altitude = tempFloat;
					}
					tempFloat = strtof(lineArray[11], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->geoidSep = tempFloat;
					}
					temp8 = strtol(lineArray[13], &error_ptr, 10);
					if (*error_ptr == '\0') {
						gnssData->diffTime = temp8;
					}
					tempFloat = strtol(lineArray[14], &error_ptr, 10);
					if (*error_ptr == '*') {
						gnssData->diffID = temp8;
					}
					status = 0;
				}
			} else if (strncmp("RMC", lineArray[0] + 3, 3) == 0) {
				if (lineArray[2][0] == 'A' && lineArray[12][0] != 'N') {
					temp32 = strtol(lineArray[1], &error_ptr, 10);
					if (*error_ptr == '.') {
						gnssData->GPStime = temp32;
						status = 0;
					}
					convertToDegree(lineArray[3], lineArray[4][0],
							&(gnssData->latitude));
					convertToDegree(lineArray[5], lineArray[6][0],
							&(gnssData->longitude));
					tempFloat = strtof(lineArray[7], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->speed = tempFloat;
					}
					tempFloat = strtof(lineArray[8], &error_ptr);
					if (*error_ptr == '\0') {
						gnssData->angle = tempFloat;
					}
					temp32 = strtol(lineArray[9], &error_ptr, 10);
					if (*error_ptr == '\0') {
						gnssData->date = temp32;
					}
					tempFloat = strtof(lineArray[10], &error_ptr);
					if (*error_ptr == '\0') {
						if (lineArray[11][0] == 'W') {
							tempFloat *= -1;
						}
						gnssData->magnetic = tempFloat;
					}
					char *quality = getQuality(lineArray[12][0]);
					if (quality[0] != '\0') {
						strcpy(gnssData->quality, quality);
					}
					status = 0;
				} else if (lineArray[1][0] != '\0') { //If time still available
					temp32 = strtol(lineArray[1], &error_ptr, 10);
					if (*error_ptr == '.') {
						gnssData->GPStime = temp32;
						status = 0;
					}
				}
			} else if (strncmp("GLL", lineArray[0] + 3, 3) == 0) {
				if (lineArray[6][0] == 'A') {
					convertToDegree(lineArray[1], lineArray[2][0],
							&(gnssData->latitude));
					convertToDegree(lineArray[3], lineArray[4][0],
							&(gnssData->longitude));
					temp32 = strtol(lineArray[5], &error_ptr, 10);
					if (*error_ptr == '.') {
						gnssData->GPStime = temp32;
					}
					status = 0;
				} else if (lineArray[5][0] != '\0') { //If time still available
					temp32 = strtol(lineArray[5], &error_ptr, 10);
					if (*error_ptr == '.') {
						gnssData->GPStime = temp32;
						status = 0;
					}
				}
			} else {
				// Reformat into original and print
				printf("Cannot parse line: ");
				for (i = 0; i < tokenNum - 1; i++) {
					printf("%s,", lineArray[i]);
				}
				printf("%s\r\n", lineArray[i + 1]);
			}

		}
		HAL_Delay(100);
		lineCount++;
		if (lineCount < numOfLines) {
			line = lines[lineCount];
			//printf("Next line: %s\r\n", line);
		} else {
			line = NULL;
			//printf("END OF DATA\r\n");
		}
	}

	if (status) {
		printf("No valid data\n");
		free(gnssData);
		gnssData = NULL;
	}

	free(*lines);
	free(lines);

	return gnssData;
}

char * getOpMode(char type)
{
	switch (type)
	{
		case 'M':
			return "Manual";
		case 'A':
			return "Automatic";
		default:
			return "Unknown";
	}
}

char * getID(char type)
{
	switch(type)
	{
		case '1':
			return "GPS";
		case '2':
			return "GLONASS";
		case '3':
			return "GALILEO";
		case '5':
			return "BEIDOU";
		default:
			return "Unknown";
	}
}

/**
 * Captures important data from the given lineArr and sets the relevant
 * gnssData struct fields for the GSA sentence.
 * 
 * @param lineArr The line array that contains all information for the sentence
 * @param length The length of the line array
 * @param gnssData The struct in which to place the captured data
 * 
 * @returns An error code depending on which field was not correctly converted. See below:
 * 
 * 		00000000 -> No error, the data was captured successfully.
 * 		00000001 -> There was no fix for this sentence, hence no useful data.
 * 		00000010 -> The PDOP field was not correctly converted.
 * 		00000100 -> The HDOP field was not correctly converted.
 * 		00001000 -> The VDOP field was not correctly converted.
 * 
 * 		The error code may be a logical OR of any of these flags.
 */
uint8_t captureGSAData(char ** lineArr, uint16_t length, GNSSData * gnssData)
{
	char * unconverted;
	float_t converted;
	uint8_t errCode = 0;

	/* If the navMode has no fix, return */
	if (*lineArr[2] == '1')
	{
		return errCode | 1;
	}

	strcpy(gnssData->quality, getOpMode(*lineArr[1]));

	converted = strtof(lineArr[15], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->PDOP = converted;
	}
	else {errCode |= 1 << 2;}


	converted = strtof(lineArr[16], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->HDOP = converted;
	}
	else {errCode |= 1 << 3;}

	
	converted = strtof(lineArr[17], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->VDOP = converted;
	}
	else {errCode |= 1 << 4;}

	strcpy(gnssData->ID, getID(*lineArr[18]));

	return errCode;
}

/**
 * Captures important data from the given lineArr and sets the relevant
 * gnssData struct fields for the VTG sentence.
 * 
 * @param lineArr The line array that contains all information for the sentence
 * @param length The length of the line array
 * @param gnssData The struct in which to place the captured data
 * 
 * @returns An error code depending on which field was not correctly converted. See below:
 * 
 * 		00000000 -> No error, the data was captured successfully.
 * 		00000001 -> The course over ground (true) was not correctly converted.
 * 		00000010 -> The course over ground (magnetic) was not correctly converted.
 * 		00000100 -> The speed over ground was not correctly converted.
 * 		00001000 -> The mode indicator could not be correctly determined.
 * 
 * 		The error code may be a logical OR of any of these flags.
 */
uint8_t captureVTGData(char ** lineArr, uint16_t length, GNSSData * gnssData)
{
	char * unconverted;
	float_t converted;
	uint8_t errCode = 0;

	converted = strtof(lineArr[1], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->angle = converted;
	}
	else {errCode |= 1;}


	converted = strtof(lineArr[3], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->magnetic = converted;
	}
	else {errCode |= 1 << 2;}


	converted = strtof(lineArr[5], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->speed = converted;
	}
	else {errCode |= 1 << 3;}


	char *quality = getQuality(*lineArr[9]);
	if (*quality != '\0')
	{
		strcpy(gnssData->quality, quality);
	}
	else {errCode |= 1 << 4;}


	return errCode;
}

/**
 * Captures important data from the given lineArr and sets the relevant
 * gnssData struct fields for the GGA sentence.
 * 
 * @param lineArr The line array that contains all information for the sentence
 * @param length The length of the line array
 * @param gnssData The struct in which to place the captured data
 * 
 * @returns An error code depending on which field was not correctly converted. See below:
 * 
 * 		0x0000 -> No error, the data was captured successfully.
 * 		0x0001 -> There was no fix for this sentence, hence no useful data.
 * 		0x0002 -> The time was not correctly converted.
 *		0x0004 -> The quality could not be correctly determined.
 * 		0x0008 -> The number of satellites could not be correctly determined.
 * 		0x0010 -> The HDOP field was not correctly converted.
 * 		0x0020 -> The altitude was not correctly converted.
 * 		0x0040 -> The geoid separation was not correctly converted.
 * 		0x0080 -> The age of differential stations could not be correctly determined.
 * 		0x0100 -> The ID of the differential station could not be correctly determined.
 * 
 * 		The error code may be a logical OR of any of these flags.
 */
uint16_t captureGGAData(char ** lineArr, uint16_t length, GNSSData * gnssData)
{
	char * unconverted;
	float_t converted;
	uint32_t convertedLong;
	uint8_t errCode = 0;

	/* If the quality has no fix, return */
	if (*lineArr[6] == '0')
	{
		return errCode | 1;
	}

	
	convertedLong = strtol(lineArr[1], &unconverted, 10);
	if (*unconverted == '.')
	{
		gnssData->GPStime = convertedLong;
	}
	else {errCode |= 1 << 2;}


	convertToDegree(lineArr[2], *lineArr[3],
			&(gnssData->latitude));
	convertToDegree(lineArr[4], *lineArr[5],
			&(gnssData->longitude));

	
	char *quality = getQuality(*lineArr[6]);
	if (quality[0] != '\0')
	{
		strcpy(gnssData->quality, quality);
	}
	else {errCode |= 1 << 3;}


	convertedLong = strtol(lineArr[7], &unconverted, 10);
	if (*unconverted == '\0')
	{
		gnssData->satellites = (uint8_t) convertedLong;
	}
	else {errCode |= 1 << 4;}


	converted = strtof(lineArr[8], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->HDOP = converted;
	}
	else {errCode |= 1 << 5;}


	converted = strtof(lineArr[9], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->altitude = converted;
	}
	else {errCode |= 1 << 6;}


	converted = strtof(lineArr[11], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->geoidSep = converted;
	}
	else {errCode |= 1 << 7;}


	convertedLong = strtol(lineArr[13], &unconverted, 10);
	if (*unconverted == '\0')
	{
		gnssData->diffTime = (uint8_t) convertedLong;
	}
	else {errCode |= 1 << 8;}


	convertedLong = strtol(lineArr[14], &unconverted, 10);
	if (*unconverted == '*')
	{
		gnssData->diffID = (uint8_t) convertedLong;
	}
	else {errCode |= 1 << 9;}
	

	return errCode;
}

/**
 * Captures important data from the given lineArr and sets the relevant
 * gnssData struct fields for the RMC sentence.
 * 
 * @param lineArr The line array that contains all information for the sentence
 * @param length The length of the line array
 * @param gnssData The struct in which to place the captured data
 * 
 * @returns An error code depending on which field was not correctly converted. See below:
 * 
 * 		00000000 -> No error, the data was captured successfully.
 * 		00000001 -> The data provided was not valid.
 * 		00000010 -> The time was not correctly converted.
 *		00000100 -> There was no position fix.
 * 		00001000 -> The speed over ground could not be correctly determined.
 * 		00010000 -> The course over ground could not be correctly determined.
 * 		00100000 -> The date was not correctly converted.
 * 		01000000 -> The magnetic variation could not be correctly determined.
 * 		10000000 -> The position mode could not be correctly determined.
 * 
 * 		The error code may be a logical OR of any of these flags.
 */
uint8_t captureRMCData(char ** lineArr, uint16_t length, GNSSData * gnssData)
{
	char * unconverted;
	float_t converted;
	uint32_t convertedLong;
	uint8_t errCode = 0;

	/* If the data is not valid, return */
	if (*lineArr[2] != 'A')
	{
		return errCode | 1;
	}
	
	/* If the data has a time fix, save the time fix */
	convertedLong = strtol(lineArr[1], &unconverted, 10);
	if (*unconverted == '.')
	{
		gnssData->GPStime = convertedLong;
	}
	else {errCode |= 1 << 2;}

	/* If the data has no position fix, return */
	if (*lineArr[12] == 'N')
	{
		return errCode | 1 << 3;
	}

	
	convertToDegree(lineArr[3], *lineArr[4],
			&(gnssData->latitude));
	convertToDegree(lineArr[5], *lineArr[6],
			&(gnssData->longitude));
	
	
	converted = strtof(lineArr[7], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->speed = converted;
	}
	else {errCode |= 1 << 4;}


	converted = strtof(lineArr[8], &unconverted);
	if (*unconverted == '\0')
	{
		gnssData->angle = converted;
	}
	else {errCode |= 1 << 5;}


	convertedLong = strtol(lineArr[9], &unconverted, 10);
	if (*unconverted == '\0')
	{
		gnssData->date = convertedLong;
	}
	else {errCode |= 1 << 6;}


	/* Convert the magnetic variation to a float - negative if the direction is West */
	converted = strtof(lineArr[10], &unconverted);
	if (*unconverted == '\0')
	{
		if (*lineArr[11] == 'W')
		{
			converted *= -1;
		}
		gnssData->magnetic = converted;
	}
	else {errCode |= 1 << 7;}


	char *quality = getQuality(*lineArr[12]);
	if (quality[0] != '\0')
	{
		strcpy(gnssData->quality, quality);
	}
	else {errCode |= 1 << 8;}
	

	return errCode;
}

/**
 * Captures important data from the given lineArr and sets the relevant
 * gnssData struct fields for the GLL sentence.
 * 
 * @param lineArr The line array that contains all information for the sentence
 * @param length The length of the line array
 * @param gnssData The struct in which to place the captured data
 * 
 * @returns An error code depending on which field was not correctly converted. See below:
 * 
 * 		00000000 -> No error, the data was captured successfully.
 * 		00000001 -> The position data was not correctly converted.
 * 		00000010 -> The time data was not correctly converted.
 * 
 * 		The error code may be a logical OR of any of these flags.
 */
uint8_t captureGLLData(char ** lineArr, uint16_t length, GNSSData * gnssData)
{
	char * unconverted;
	uint32_t converted;
	uint8_t errCode = 0;

	
	if (*lineArr[6] == 'A')
	{
		convertToDegree(lineArr[1], *lineArr[2],
				&(gnssData->latitude));
		convertToDegree(lineArr[3], *lineArr[4],
				&(gnssData->longitude));
		
		
		converted = strtol(lineArr[5], &unconverted, 10);
		if (*unconverted == '.')
		{
			gnssData->GPStime = converted;
		}
		else {errCode |= 1;}
	}
	else if (*lineArr[5] != '\0')
	{
		converted = strtol(lineArr[5], &unconverted, 10);
		if (*unconverted == '.')
		{
			gnssData->GPStime = converted;	
		}
		else {errCode |= 1 << 2;}
	}
	

	return errCode;
}

uint8_t nmeaChecksum(const char *data, const uint16_t length) {
	uint16_t i;
	uint8_t check = 0;
	if ((data + length - 3)[0] == '*') {
		for (i = 1; i < length - 3; i++) {
			check ^= (uint8_t) data[i];
		}
		if (check == (uint8_t) strtol(data + length - 2, NULL, 16)) {
			return 0;
		}
	}
	return -1;
}

/**
 * Verifies the format of the given NMEA message.
 *
 * @param data The message to check for the correct format
 *
 * @returns 0 if the message is of the correct format and -1 if it is not.
 * 
 * @note This will be deprecated once a regex version of this is implemented
 * 		 in c++ as currently stm32 ide does not recognise the regex.h include.
 */
int8_t verifyFormat(const char *data)
{
	uint8_t i;
	uint8_t nCorrect;
	char * found;

	if (data != NULL)
	{
		// Ensure that at least the header and checksum are included ie. $TTSSS,*XX
		if (strlen(data) >= 10)
		{
			// Ensure the message begins with '$'
			if (*data == '$')
			{
				nCorrect = 0;
		
				// Ensure that the next 5 chars are either numeric or capital letters
				for (i = 1; i < 6; i++)
				{
					if (isdigit(data[i]) || isupper(data[i]))
					{
						nCorrect++;
					}
				}
		
				if (nCorrect == 5)
				{
					// Ensure that the following character is a ','
					if (data[6] == ',')
					{
						found = strchr(data, '*');
		
						/* NOTE: Almost certainly the \r\n will end the line and should be considered part
						of the format. This will need to be rectified, but at the moment either with or
						without will pass the check.*/

						// Ensure there exists a '*' character and contains enough values for a checksum
						if (found != NULL && strlen(found) >= 3)
						{
							// Ensure the checksum is hexadecimal
							if (isxdigit(found[1]) && isxdigit(found[2]))
							{
								return 0;
							}
						}
					}
				}
			}
		}
	}

	return -1;
}

Constellation verifyData(const char *data, const uint16_t length) {
	if (length > 9) { //6 for the $GXXXX and 3 for *XX
		if (nmeaChecksum(data, length) == 0) {
			if (strncmp("$GNTXT", data, 6) == 0) {
				printf("%s\r\n", data); //Print out any GNTXT data
				return INVALID;
			} else {
				return verifyValidData(data);
			}
		} else {
			printf("Failed NMEA - ");
		}
	}
	printf("Invalid data: %s\r\n", data);
	return INVALID;
}

/**
 * Converts the given header to a constellation
 * 
 * @param header The header from which to extract the constellation in
 * 				 the format: $TTSSS
 * 
 * @returns The constellation from the header or INVALID if it cannot be
 * 			determined.
 * 
 * @note This function assumes correct format of the header as $TTSSS
 */
Constellation convertConstellation(const char *header)
{	
	static const uint8_t nConstellations = 6;
	static const char constels[6][3] = {"GP", "GL", "GA", "GB", "BD", "GN"};
	static const Constellation constellations[6] = {GPS, GLONASS, GALILEO, BEIDOU, BEIDOU, NONE};

	uint8_t i;

	for (i = 0; i < nConstellations; i++)
	{
		if (strncmp(constels[i], header + 1, 2) == 0)
		{
			return constellations[i];
		}
	}

	return INVALID;
}

Constellation verifyValidData(const char *data) {
	Constellation constellation = INVALID;
	/*char* head[4] = { 0 };
	 strncpy(head, data + 3, 3);
	 printf("%s\r\n", head);
	 if (strncmp("GSA", head, 3) == 0) {
	 printf("GNGSA!!!!\r\n");
	 } else if (strncmp("GSV", head, 3) == 0) {
	 correct = 1;
	 } else if (strncmp("VTG", head, 3) == 0) {
	 printf("GNVTG!!!!\r\n");
	 } else if (strncmp("GGA", head, 3) == 0) {
	 printf("GNGGA!!!!\r\n");
	 } else if (strncmp("RMC", head, 3) == 0) {
	 printf("GNRMC!!!!\r\n");
	 } else if (strncmp("GLL", head, 3) == 0) {
	 printf("GNGGL!!!!\r\n");
	 }*/
	if (strncmp("$GP", data, 3) == 0) {
		constellation = GPS;
	} else if (strncmp("$GL", data, 3) == 0) {
		constellation = GLONASS;
	} else if (strncmp("$GA", data, 3) == 0) {
		constellation = GALILEO;
	} else if (strncmp("$GB", data, 3) == 0 || strncmp("$BD", data, 3) == 0) {
		constellation = BEIDOU;
	} else if (strncmp("$GN", data, 3) == 0) {
		constellation = NONE;
	}
	return constellation;
}
