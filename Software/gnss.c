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
	switch (data)
	{
		case '0': case 'N':
			return "No fix";
		case '1': case 'A':
			return "Autonomous";
		case '2': case 'D':
			return "Differential";
		case '4': case 'R':
			return "RTK fixed";
		case '5': case 'F':
			return "RTK Float";
		case '6': case 'E':
			return "Estimated";
		default:
			return '\0';
	}
}

Sentences getSentenceType(const char * header)
{
	char type[4];

	strncpy(type, header + 3, 3);

	if (strcmp(type, "GSA") == 0)
	{
		return GSA;
	}
	else if (strcmp(type, "VTG") == 0)
	{
		return VTG;
	}
	else if (strcmp(type, "GGA") == 0)
	{
		return GGA;
	}
	else if (strcmp(type, "RMC") == 0)
	{
		return RMC;
	}
	else if (strcmp(type, "GLL") == 0)
	{
		return GLL;
	}
	else
	{
		return INV;
	}
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
	uint16_t nArgs = 0;

	char ** lines = splitString(data, outer_delimiter, &numOfLines);

	GNSSData *gnssData = createGNSSData();
	uint8_t status = 1;



	for (i = 0; i < numOfLines; i++)
	{
		char * line = lines[i];
		char ** lineArr;
		
		if (verifyFormat(line) == 0)
		{
			if (nmeaChecksum(line) == 0)
			{
				lineArr = splitString(line, inner_delimiter, &nArgs);

				gnssData->constellation = convertConstellation(lineArr[0]);

				if (gnssData->constellation != INVALID)
				{
					Sentences sent_type = getSentenceType(lineArr[0]);
					uint16_t errCode = 0;
					

					switch(sent_type)
					{
						case GGA:
							errCode = captureGGAData(lineArr, nArgs, gnssData);
							break;
						case GLL:
							errCode = captureGLLData(lineArr, nArgs, gnssData);
							break;
						case GSA:
							errCode = captureGSAData(lineArr, nArgs, gnssData);
							break;
						case RMC:
							errCode = captureRMCData(lineArr, nArgs, gnssData);
							break;
						case VTG:
							errCode = captureVTGData(lineArr, nArgs, gnssData);
							break;
						default:
							break;
					}

					if (errCode != 0)
					{
						printf("Could not parse the sentence: %s\n", line);
					}
					else
					{
						status = 0;
					}
				}
			}
			else
			{
				printf("Failed checksum for data: %s\n", line);
			}
		}
		else
		{
			printf("Invalid data format for data: %s\n", line);
		}
		
		if (*lineArr != NULL) free(*lineArr);
		if (lineArr != NULL) free(lineArr);
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

int8_t nmeaChecksum(const char *data) {
	uint16_t i = 1;
	uint8_t check = 0;
	char * checksumPos = strchr(data, '*');
	if (checksumPos != NULL)
	{
		while(data[i] != '*')
		{
			check ^= (uint8_t) data[i];
			i++;
		}

		if (check == (uint8_t) strtol(checksumPos + 1, NULL, 16))
		{
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
