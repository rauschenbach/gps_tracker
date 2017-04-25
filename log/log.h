#ifndef _LOG_H
#define _LOG_H


#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_defs.h"

/* Строки в лог файле  */
#define   INFO_NUM_STR  	0
#define   POS_NUM_STR		1
#define   BEGIN_REG_NUM_STR	2
#define   END_REG_NUM_STR	3
#define   BEGIN_BURN_NUM_STR	4
#define   BURN_LEN_NUM_STR	5
#define   POPUP_LEN_NUM_STR	6
#define   MODEM_NUM_NUM_STR	7
#define   ALARM_TIME_NUM_STR	8
#define   DAY_TIME_NUM_STR	9
#define   ADC_FREQ_NUM_STR	10
#define   ADC_CONSUM_NUM_STR	11
#define   ADC_PGA_NUM_STR	12
#define   MODEM_TYPE_NUM_STR	13
#define   ADC_BITMAP_NUM_STR	14
#define   FILE_LEN_NUM_STR	15
#define   FLT_FREQ_NUM_STR	16
#define   CONST_REG_NUM_STR	17



/** 
 * Здесь ощибки, которые могут быть возвражены функцией
 */
typedef enum {
	// не ошибки
	RES_NO_ERROR = 0,		// Нет ошибки
	RES_NO_LOCK_FILE,		// Нет лок файла

	RES_WRITE_UART_ERR = -4,	// Ошибка записи в порт
	RES_DEL_LOCK_ERR = -5,		// Ошибка удаления лок файла
	RES_MOUNT_ERR = -6,		// Ошибка монтирования	
	RES_FORMAT_ERR = -7,		// Ошибка формирования строки
	RES_WRITE_LOG_ERR = -8,		// Ошибка записи в лог
	RES_SYNC_LOG_ERR = -9,		// Ошибка записи в лог (sync)
	RES_CLOSE_LOG_ERR = -10,	// Ошибка закрытия лога
	RES_OPEN_DATA_ERR = -11,	// Ошибка открытия файла данных
	RES_WRITE_DATA_ERR = -12,	// Ошибка записи файла данных
	RES_CLOSE_DATA_ERR = -13,	// Ошибка закрытия файла данных
	RES_WRITE_HEADER_ERR = -14,	// Ошибка записи минутного заголовка
	RES_SYNC_HEADER_ERR = -15,	// Ошибка записи заголовка (sync)
	RES_REG_PARAM_ERR = -16,	// Ошибка в файле параметров
	RES_MALLOC_PARAM_ERR = -17,	// Ошибка выделения памяти
	RES_OPEN_PARAM_ERR = -18,	// Ошибка открытия файла параметров
	RES_READ_PARAM_ERR = -19,	// Ошибка чтения файла параметров
	RES_CLOSE_PARAM_ERR = -20,	// Ошибка закрытия файла параметров
	RES_TIME_PARAM_ERR = -21,  	// Ошибка в задании времени
	RES_FREQ_PARAM_ERR = -22,	// Ошибка в задании частоты
	RES_CONSUMP_PARAM_ERR = -23,	// Ошибка в задании енергопотребления
	RES_PGA_PARAM_ERR = -24,     	// Ошибка в задании усиления
	RES_MODEM_TYPE_ERR = -25,  	// Ошибка в задании числа байт
	RES_MKDIR_PARAM_ERR = -26,  	// Ошибка в создании папки
	RES_MAX_RUN_ERR = -27,  	// Исчерпаны запуски
	RES_CREATE_LOG_ERR = -28,  	// Ошибка создания лога
	RES_CREATE_ENV_ERR = -29,  	// Ошибка создания лога среды
	RES_READ_FLASH_ERR = -30,	// Ошибка чтения flash
	RES_OPEN_LOADER_ERR = -31,	// Ошибка открытия загрузчика
	RES_READ_LOADER_ERR = -32,	// Ошибка чтения загрузчика
	RES_CLOSE_LOADER_ERR = -33,	// Ошибка закрытия загрузчика


	RES_OPEN_BOOTFILE_ERR = -35,	// Ошибка открытия загрузчика
	RES_WRITE_BOOTFILE_ERR = -36,	// Ошибка чтения загрузчика
	RES_READ_BOOTFILE_ERR = -37,	// Ошибка чтения загрузчика
	RES_CLOSE_BOOTFILE_ERR = -38,	// Ошибка закрытия загрузчика


	RES_FORMAT_TIME_ERR = -40,	// Ошибка форматирования времени
} ERROR_ResultEn;



/*******************************************************************
*  function prototypes
*******************************************************************/

int  log_term_printf(const char *, ...);
int  log_term_out(const char *fmt, ...);
int log_create_log_file(void);

int  log_mount_fs(void);
int  log_write_debug_str(char *, ...);
int  log_write_log_to_uart(char *, ...);
bool log_check_mounted(void);
int  log_write_log_file(char *, ...);

#endif /* log.h */
