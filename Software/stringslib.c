#include "stringslib.h"

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

	/* If the string is not longer than the token, return */
	if (strlen(string) < tokenLength)
	{
		return 0;
	}

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

	/* If the string length is not greater than the delimiter length, return NULL */
	if (strlen(string) <= strlen(delim))
	{
		free(input);
		return NULL;
	}

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