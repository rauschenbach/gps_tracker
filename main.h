#ifndef _MAIN_H
#define _MAIN_H

/* Library includes. */
#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include "sdio_drv.h"
#include "ff.h"

#include "my_defs.h"
#include "usart.h"
#include "board.h"
#include "circbuf.h"
#include "utils.h"
#include "gps.h"
#include "gsm.h"
#include "led.h"
#include "FreeRTOS.h"

/* Scheduler includes. */
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stackmacros.h"

static uint32_t SystemFrequency = 24000000;

#define TIME_DELAY_5_MS		5
#define DELAY_CMD_MS		250
#define CONNECT_TIMEOUT		2000
#define WAIT_TIME_2SEC		CONNECT_TIMEOUT
#define WAIT_TIME_10SEC		10000
#define WAIT_TIME_20SEC		20000


#define SERIAL_NUM		184
//#define SERVER_STRING		"AT+CIPSTART=\"TCP\",\"87.249.0.254\",\"10030\"\r\n"
//#define SERVER_STRING		"AT+CIPSTART=\"TCP\",\"87.249.0.254\",\"10024\"\r\n"
#define SERVER_STRING		"AT+CIPSTART=\"UDP\",\"87.249.0.254\",\"10025\"\r\n"


#define INVALID_QUEUE				((xQueueHandle) 0)


#endif /* main.h */
