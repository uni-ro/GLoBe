#include "buffer.h"

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
uint8_t * buffNext(uint8_t ** buffer, const uint8_t * delim, size_t delimLength, size_t bufferStart, size_t bufferLength)
{
	uint8_t * data = *buffer;
	uint8_t * found;

	found = findInBuff(data, delim, delimLength, bufferStart, bufferLength);

	if (found != NULL)
	{
		*found = '\0';
		*buffer = found + delimLength;
	}

	return data;
}

// NOTE: Will not find the needle if bufferStart has split it.
uint8_t * findInBuff(uint8_t * haystack, const uint8_t * needle, size_t needleLength, size_t bufferStart, size_t bufferLength)
{
	uint8_t * found = NULL;
	uint8_t * searchLoc = NULL;
	uint8_t currByte = 0;

	size_t i = 0;
	size_t j = 0;

	/* Scan the string for instances of the needle, exiting when found */
	for (i = 0; i < bufferLength - needleLength + 1; i++)
	{
		for (j = 0; j < needleLength; j++)
		{
			searchLoc = haystack + ((i + bufferStart + j) % bufferLength);

			currByte = *searchLoc;

			if (currByte != needle[j])
			{
				found = NULL;
				break;
			}

			found = haystack + (i + bufferStart) % bufferLength;
		}
		
		if (found != NULL)
		{
			break;
		}
	}
	
	return found;
}

// NOTE: destLength must be less than buffer length
// NOTE: The returned array must be freed afterwards
uint8_t * copyFromBuff(uint8_t * buffer, size_t destLength, uint8_t * needle, size_t needleLength, size_t bufferStart, size_t bufferLength)
{
    uint8_t * copy = NULL;
    uint8_t * found = findInBuff(buffer, needle, needleLength, bufferStart, bufferLength);
    uint8_t i = 0;

    if (found != NULL)
    {
        copy = (uint8_t *) malloc(sizeof(uint8_t) * destLength);

        for (i = 0; i < destLength; i++)
        {
            copy[i] = buffer[(found - buffer + i) % bufferLength];
        }
    }

    return copy;
}

/**
 * NOTE: Assumes contiguous data in buffer.
 */
uint16_t numInstances(const uint8_t * buffer, size_t bufferLength, const uint8_t * delim, size_t delimLength)
{
	uint16_t nTokens = 0;
	uint16_t i = 0;

	/* If the buffer is not longer than the delimiter, return */
	if (bufferLength < delimLength)
	{
		return 0;
	}

	/* Scan the string for instances of the delim, ending with the final delim size */
	for (i = 0; i < bufferLength - delimLength + 1; i++)
	{
		if (memcmp(delim, buffer + i, delimLength) == 0)
		{
			nTokens++;
		}
	}

	return nTokens;
}

/**
 * Splits a given buffer into an array of uint8_t pointers. (Generalisation of `splitString`)
 * 
 * @note This is different to `splitString` in that it returns an array of size n for n delimiters in 
 * 		 the buffer including if the delimiter is at the end of the string / byte arr.
 */
uint8_t ** splitBuff(const uint8_t * buffer, size_t bufferLength, size_t startIdx, const uint8_t * delim, size_t delimLength, uint16_t * arr_size)
{
	/* Create copy of input string for strsep to use */
	uint8_t * input = (uint8_t *) calloc(bufferLength + 1, sizeof(uint8_t));
	
	/* Copy from buffer to input so that the data is contiguous from the startIdx. ie. maps startIdx -> 0 (of input)*/
	memcpy(input, buffer + startIdx, bufferLength - startIdx);
	memcpy(input + bufferLength - startIdx, buffer, startIdx);
	
	uint16_t nParts = numInstances(buffer, bufferLength, delim, delimLength) + 1; /* For n splits, there are n+1 sections */

	/* If the buffer length is not greater than the delimiter length, return NULL */
	if (bufferLength <= delimLength)
	{
		free(input);
		return NULL;
	}

	/* If the buffer ends with the delimiter, create only n sections
	 * For example, Foo\r with delimiter as '\r' returns an array of size 2.
	 * 		Element 0 is: Foo 
	 * 		Element 1 is: \0
	 * 		Hence, the array size should be corrected to 1 instead of 2.
	 */
	// if (memcmp(input + bufferLength - delimLength, delim, delimLength) == 0)
	// {
	// 	nParts--;
	// }
	// For buffer reasons, it is preferable to have the extra split at the end.

	uint8_t ** data = (uint8_t **) calloc(nParts, sizeof(uint8_t *));

	uint8_t * currBuffPtr = input;

    uint16_t i = 0;

	for (i = 0; i < nParts; i++)
	{
		data[i] = currBuffPtr;
		
		uint8_t * found;

		found = findInBuff(input, delim, delimLength, currBuffPtr - input, bufferLength);

		if (found != NULL)
		{
			*found = '\0';
			currBuffPtr = found + delimLength;
		}
	}

    *arr_size = nParts;


	return data;
}