#ifndef ERROR_H
#define ERROR_H
typedef enum ERROR_CODE
{
	E_SUCCESS = 0,
	E_FAILURE = -1,
	E_INVALID_INPUT = -2,
	E_FILE_NOT_FOUND = -3, 
	E_OUT_OF_MEMORY = -4,
	E_EMPTY_LIST = -5
} ERROR_CODE;

char* getErrorName( ERROR_CODE code);
#endif