#ifndef _LOG_H
#define _LOG_H


#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "my_defs.h"

/* ������ � ��� �����  */
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
 * ����� ������, ������� ����� ���� ���������� ��������
 */
typedef enum {
	// �� ������
	RES_NO_ERROR = 0,		// ��� ������
	RES_NO_LOCK_FILE,		// ��� ��� �����

	RES_WRITE_UART_ERR = -4,	// ������ ������ � ����
	RES_DEL_LOCK_ERR = -5,		// ������ �������� ��� �����
	RES_MOUNT_ERR = -6,		// ������ ������������	
	RES_FORMAT_ERR = -7,		// ������ ������������ ������
	RES_WRITE_LOG_ERR = -8,		// ������ ������ � ���
	RES_SYNC_LOG_ERR = -9,		// ������ ������ � ��� (sync)
	RES_CLOSE_LOG_ERR = -10,	// ������ �������� ����
	RES_OPEN_DATA_ERR = -11,	// ������ �������� ����� ������
	RES_WRITE_DATA_ERR = -12,	// ������ ������ ����� ������
	RES_CLOSE_DATA_ERR = -13,	// ������ �������� ����� ������
	RES_WRITE_HEADER_ERR = -14,	// ������ ������ ��������� ���������
	RES_SYNC_HEADER_ERR = -15,	// ������ ������ ��������� (sync)
	RES_REG_PARAM_ERR = -16,	// ������ � ����� ����������
	RES_MALLOC_PARAM_ERR = -17,	// ������ ��������� ������
	RES_OPEN_PARAM_ERR = -18,	// ������ �������� ����� ����������
	RES_READ_PARAM_ERR = -19,	// ������ ������ ����� ����������
	RES_CLOSE_PARAM_ERR = -20,	// ������ �������� ����� ����������
	RES_TIME_PARAM_ERR = -21,  	// ������ � ������� �������
	RES_FREQ_PARAM_ERR = -22,	// ������ � ������� �������
	RES_CONSUMP_PARAM_ERR = -23,	// ������ � ������� �����������������
	RES_PGA_PARAM_ERR = -24,     	// ������ � ������� ��������
	RES_MODEM_TYPE_ERR = -25,  	// ������ � ������� ����� ����
	RES_MKDIR_PARAM_ERR = -26,  	// ������ � �������� �����
	RES_MAX_RUN_ERR = -27,  	// ��������� �������
	RES_CREATE_LOG_ERR = -28,  	// ������ �������� ����
	RES_CREATE_ENV_ERR = -29,  	// ������ �������� ���� �����
	RES_READ_FLASH_ERR = -30,	// ������ ������ flash
	RES_OPEN_LOADER_ERR = -31,	// ������ �������� ����������
	RES_READ_LOADER_ERR = -32,	// ������ ������ ����������
	RES_CLOSE_LOADER_ERR = -33,	// ������ �������� ����������


	RES_OPEN_BOOTFILE_ERR = -35,	// ������ �������� ����������
	RES_WRITE_BOOTFILE_ERR = -36,	// ������ ������ ����������
	RES_READ_BOOTFILE_ERR = -37,	// ������ ������ ����������
	RES_CLOSE_BOOTFILE_ERR = -38,	// ������ �������� ����������


	RES_FORMAT_TIME_ERR = -40,	// ������ �������������� �������
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
