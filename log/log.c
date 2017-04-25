/* Сервисные функции: ведение лога, запись на SD карту */
#include <ff.h>
#include <diskio.h>
#include "sdio_drv.h"
#include "board.h"
#include "utils.h"
#include "log.h"
#include "main.h"
#include "minIni.h"



#define   	MAX_FILE_SIZE		256
#define   	MAX_START_NUMBER	100	/* Максимальное число запусков */
#define	  	MAX_TIME_STRLEN		18	/* Длина строки со временем  */
#define   	MAX_LOG_FILE_LEN	134217728	/* 128 Мбайт */
#define   	MAX_FILE_NAME_LEN	31	/* Длина файла включая имя директории с '\0' */
#define		PARAM_FILE_NAME		"recparam.ini"
#define 	ERROR_LOG_NAME		"error.log"	/* Файл ошибок  */
#define 	BOOT_LOG_NAME		"boot.log"	/* Файл лога загрузки */


/************************************************************************
 *      Эти статические переменные не видимы в других файлах 
 * 	отображаются или указателем SRAM (USE_THE_LOADER)
 * 	или используются в секции данных  
 ************************************************************************/
static FATFS fatfs;		/* File system object - можно убрать из global? нет! */
static DIR dir;			/* Директория где храница все файло - можно убрать из global? */
static FIL log_file;		/* File object */


/**
 * Вывод на экран, для отладки
 */
int log_term_printf(const char *fmt, ...)
{
    char buf[256];
    int ret;
    va_list list;

    /* Получаем текущее время - MAX_TIME_STRLEN символов с пробелом - всегда пишем */
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

/* Вывод без времени */
int log_term_out(const char *fmt, ...)
{
    char buf[256];
    int ret;
    va_list list;

    /* Получаем текущее время - MAX_TIME_STRLEN символов с пробелом - всегда пишем */
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
 * Инициализация файловой системы
 * При монтировании читаем данные с FLASH
 */
int log_mount_fs(void)
{
    int res;

    res = f_mount(0, &fatfs);	/* Монтируем ФС */
    return res;
}

/**
 * Карта монтирована или нет?
 */
bool log_check_mounted(void)
{
    return fatfs.fs_type;
}


/**
 * Открыть файл лога в этой директории всегда, создать если нет!
 */
int log_create_log_file(void)
{
  int res; 

  res = f_open(&log_file, "tracker.log", FA_WRITE | FA_READ | FA_OPEN_ALWAYS);

  return res;
}

/**
 * Запись строки в лог файл, возвращаем сколько записали. С временем ВСЕГДА!
 * Режем по 10 мБайт?
 */
int log_write_log_file(char *fmt, ...)
{
    char str[256];
    FRESULT res;		/* Result code */
    unsigned bw;		/* Прочитано или записано байт  */
    int i, ret = 0;
    va_list p_vargs;		/* return value from vsnprintf  */

    /* Не монтировано (нет фс - работаем от PC), или с карточкой проблемы */
    if (&log_file.fs == NULL) {
	return RES_MOUNT_ERR;
    }

    /* Получаем текущее время - MAX_TIME_STRLEN символов с пробелом - всегда пишем */
    time_to_str(str);		// Получить время всегда
    va_start(p_vargs, fmt);
    i = vsnprintf(str + MAX_TIME_STRLEN, sizeof(str), fmt, p_vargs);
    va_end(p_vargs);
    if (i < 0) {			/* formatting error?            */
	return RES_FORMAT_ERR;
    }

    /* Заменим переносы строки на UNIX (не с начала!) */
    for (i = MAX_TIME_STRLEN + 4; i < sizeof(str) - 3; i++) {
	if (str[i] == 0x0d || str[i] == 0x0a) {
	    str[i] = 0x0d;	// перевод строки
	    str[i + 1] = 0x0a;	// Windows
	    str[i + 2] = 0;
	    break;
	}
    }

    /* Отключать запись по irq, когда мы пишем в log */
//vvv:
     i = strlen(str);
    res = f_write(&log_file, str, i, &bw);
    if (res) {
	return RES_WRITE_LOG_ERR;
    }

    /* Обязательно запишем! */
    res = f_sync(&log_file);
    if (res) {
	return RES_SYNC_LOG_ERR;
    }

    /* Включать запись по irq */
    return ret;
}


/**
 * Закрыть лог-файл
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
