#include "error.h"

/**
 * Method:    getErrorName
 * @brief     Generates the string version of the error code
 * @param 	  code - ERROR_CODE enum value to convert
 * @return    char* string description, such as "Success"
 **/
char* getErrorName( ERROR_CODE code)
{
	char* string;
	switch (code)
	{
	case E_SUCCESS: 
		string = "Success";
		break;
	case E_FAILURE:
		string = "Failure";
		break;
	case E_INVALID_INPUT:
		string = "Invalid Input";
		break;
	case E_FILE_NOT_FOUND:
		string = "File Not Found";
		break;
	case E_OUT_OF_MEMORY:
		string = "Out Of Memory";
		break;
	case E_EMPTY_LIST:
		string = "Empty List";
		break;
	default:
		string = "INVALID ENUM CODE";
	}

	return string;
}