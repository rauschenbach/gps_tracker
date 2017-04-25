/* ��������� �������: ������� ����, ������ �� SD ����� */
#include <ff.h>
#include <diskio.h>
#include "sdio_drv.h"
#include "board.h"
#include "utils.h"
#include "log.h"
#include "main.h"
#include "minIni.h"



#define   	MAX_FILE_SIZE		256
#define   	MAX_START_NUMBER	100	/* ������������ ����� �������� */
#define	  	MAX_TIME_STRLEN		18	/* ����� ������ �� ��������  */
#define   	MAX_LOG_FILE_LEN	134217728	/* 128 ����� */
#define   	MAX_FILE_NAME_LEN	31	/* ����� ����� ������� ��� ���������� � '\0' */
#define		PARAM_FILE_NAME		"recparam.ini"
#define 	ERROR_LOG_NAME		"error.log"	/* ���� ������  */
#define 	BOOT_LOG_NAME		"boot.log"	/* ���� ���� �������� */


/************************************************************************
 *      ��� ����������� ���������� �� ������ � ������ ������ 
 * 	������������ ��� ���������� SRAM (USE_THE_LOADER)
 * 	��� ������������ � ������ ������  
 ************************************************************************/
static FATFS fatfs;		/* File system object - ����� ������ �� global? ���! */
static DIR dir;			/* ���������� ��� ������� ��� ����� - ����� ������ �� global? */
static FIL log_file;		/* File object */


/**
 * ����� �� �����, ��� �������
 */
int log_term_printf(const char *fmt, ...)
{
    char buf[256];
    int ret;
    va_list list;

    /* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
    time_to_str(buf);
    va_start(list, fmt);
    ret = vsnprintf(buf + MAX_TIME_STRLEN, sizeof(buf), fmt, list);
    va_end(list);

    if (ret < 0) {
	return ret;
    }

    PRINTF("%s", buf);
    return ret;
}

/* ����� ��� ������� */
int log_term_out(const char *fmt, ...)
{
    char buf[256];
    int ret;
    va_list list;

    /* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
    time_to_str(buf);
    va_start(list, fmt);
    ret = vsnprintf(buf, sizeof(buf), fmt, list);
    va_end(list);

    if (ret < 0) {
	return ret;
    }

    PRINTF("%s", buf);
    return ret;
}


/**
 * ������������� �������� �������
 * ��� ������������ ������ ������ � FLASH
 */
int log_mount_fs(void)
{
    int res;

    res = f_mount(0, &fatfs);	/* ��������� �� */
    return res;
}

/**
 * ����� ����������� ��� ���?
 */
bool log_check_mounted(void)
{
    return fatfs.fs_type;
}


/**
 * ������� ���� ���� � ���� ���������� ������, ������� ���� ���!
 */
int log_create_log_file(void)
{
  int res; 

  res = f_open(&log_file, "tracker.log", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);

  return res;
}

/**
 * ������ ������ � ��� ����, ���������� ������� ��������. � �������� ������!
 * ����� �� 10 �����?
 */
int log_write_log_file(char *fmt, ...)
{
    char str[256];
    FRESULT res;		/* Result code */
    unsigned bw;		/* ��������� ��� �������� ����  */
    int i, ret = 0;
    va_list p_vargs;		/* return value from vsnprintf  */

    /* �� ����������� (��� �� - �������� �� PC), ��� � ��������� �������� */
    if (&log_file.fs == NULL) {
	return RES_MOUNT_ERR;
    }

    /* �������� ������� ����� - MAX_TIME_STRLEN �������� � �������� - ������ ����� */
    time_to_str(str);		// �������� ����� ������
    va_start(p_vargs, fmt);
    i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);
    va_end(p_vargs);
    if (i < 0) {			/* formatting error?            */
	return RES_FORMAT_ERR;
    }

    /* ������� �������� ������ �� UNIX (�� � ������!) */
    for (i = MAX_TIME_STRLEN + 4; i < sizeof(str) - 3; i++) {
	if (str[i] == 0x0d || str[i] == 0x0a) {
	    str[i] = 0x0d;	// ������� ������
	    str[i + 1] = 0x0a;	// Windows
	    str[i + 2] = 0;
	    break;
	}
    }

    /* ��������� ������ �� irq, ����� �� ����� � log */
//vvv:
     i = strlen(str);
    res = f_write(&log_file, str, i, &bw);
    if (res) {
	return RES_WRITE_LOG_ERR;
    }

    /* ����������� �������! */
    res = f_sync(&log_file);
    if (res) {
	return RES_SYNC_LOG_ERR;
    }

    /* �������� ������ �� irq */
    return ret;
}


/**
 * ������� ���-����
 */
int log_close_log_file(void)
{
    FRESULT res;		/* Result code */

    res = f_close(&log_file);
    if (res) {
	return RES_CLOSE_LOG_ERR;
    }
    return RES_NO_ERROR;
}
