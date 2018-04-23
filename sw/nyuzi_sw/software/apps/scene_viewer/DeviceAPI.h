#ifndef __DEVICE_COM_H
#define __DEVICE_COM_H

#include <stdint.h>

typedef enum{
	OP_START,
	OP_STOP,
	OP_ROTATE,
	OP_COUNT
} operation_t;

typedef struct 
{
	uint8_t u8Operation;
	uint8_t u8Argument;
	uint16_t u16Argument;
	uint32_t u32Argument;
} deviceCall_t;



#endif


