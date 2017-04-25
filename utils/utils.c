/******************************************************************************
 * Функции перевода дат, проверка контрольной суммы т.ж. здесь 
 * Все функции считают время от начала Эпохи (01-01-1970)
 * Все функции с маленькой буквы
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "utils.h"

/* Максимальное число наборов данных (по 6 байт в каждом)  */
#define MAX_NUM_MATS		14
#define DATA_SLOT_SIZE		6


typedef s32 xtime_t;

#define _TBIAS_DAYS		((70 * (u32)365) + 17)
#define _TBIAS_SECS		(_TBIAS_DAYS * (xtime_t)86400)
#define	_TBIAS_YEAR		1900
#define MONTAB(year)		((((year) & 03) || ((year) == 0)) ? mos : lmos)
#define	Daysto32(year, mon)	(((year - 1) / 4) + MONTAB(year)[mon])

static const s16 lmos[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };
static const s16 mos[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

/* преобразование в unix-time - правильное! */
static xtime_t right_mktime(struct tm *t)
{				/* convert time structure to scalar time */
    s32 days;
    xtime_t secs;
    s32 mon, year;

    /* Calculate number of days. */
    mon = t->tm_mon;
    year = t->tm_year - _TBIAS_YEAR + 1900;
    days = Daysto32(year, mon) - 1;
    days += 365 * year;
    days += t->tm_mday;
    days -= _TBIAS_DAYS;

    /* Calculate number of seconds. */
    secs = 3600 * t->tm_hour;
    secs += 60 * t->tm_min;
    secs += t->tm_sec;
    secs += (days * (xtime_t) 86400);
    return (secs);
}


/**
 * Переводит секунды (time_t) с начала Эпохи в формат struct tm
 */
int sec_to_tm(long ls, struct tm *time)
{
    struct tm *tm_ptr;

    if ((int) ls != -1 && time != NULL) {
	tm_ptr = gmtime((time_t *) & ls);

	/* Записываем дату, что получилось */
	memcpy(time, tm_ptr, sizeof(struct tm));
	return 0;
    } else
	return -1;
}

/**
 * Время struct tm в секунды 
 */
long tm_to_sec(struct tm *tm_time)
{
    long r;

    /* Делаем секунды */
    r = mktime(tm_time);
    return r;			/* -1 или нормальное число  */
}


/**
 * Записываем дату в формате: 08-11-15 - 08:57:22 
 */
int sec_to_str(long ls, char *str)
{
    struct tm t0;
    int res = 0;

    if (sec_to_tm((long)ls, &t0) != -1) {
	sprintf(str, "%02d-%02d-%02d-%02d:%02d:%02d",	
		 t0.tm_mday, t0.tm_mon + 1, (t0.tm_year + 1900) % 100,
		 t0.tm_hour, t0.tm_min, t0.tm_sec);
    } else {
	sprintf(str, "[set time error]");
	res = -1;
    }
    return res;
}


/**
 * Для записи времени в лог - получить время, если таймер1 не запущен, то от RTC
 */
void time_to_str(char *str)
{
    struct tm t;
    s32 sec;

    sec = board_get_sec();	/* Получаем время от таймера */

    /* Записываем дату в формате: P: 09-07-16 13:11:39 */
    if (sec_to_tm(sec, &t) != -1) {
	sprintf(str, "%02d-%02d-%02d %02d:%02d:%02d ", 
			t.tm_mday, t.tm_mon, t.tm_year % 100 ,
			t.tm_hour, t.tm_min, t.tm_sec);
    } else {
	sprintf(str, "set time error ");
    }
}


/* Печатать данные GPS для проверки */
void print_nmea_data(void *v)
{
    struct tm *t0;
    gps_data_t *data;
    char *str_fix[] = { "No Fix", "GSM Data", "2D Fix", "3D Fix" };

    data = (gps_data_t *) v;
    t0 = gmtime((time_t *) & data->time);

/* Записываем дату, что получилось */
    PRINTF("Time:\t\t%02d:%02d:%02d %02d.%02d.%02d\r\n", t0->tm_hour, t0->tm_min, t0->tm_sec, t0->tm_mday, t0->tm_mon + 1, abs(t0->tm_year - 100));
    PRINTF("Status:\t%s\r\n", str_fix[data->status % 4]);
    PRINTF("lat:\t\t%04.04f\nlon:\t\t%04.04f\nvel:\t\t%04d\nhi:\t\t%04d\r\n\n", data->lat, data->lon, data->vel, data->hi);
}

/*****************************************************************************
 *    расчет контрольной суммы
 *****************************************************************************/
long mk_crc16(long crc, void *adr, u32 len)
{
    u8 *ptr = (u8 *) adr;
    long tmpcrc = crc;
    char i;
    char ch;

    while (len--) {
	ch = *ptr++;
	for (i = 8; i--;) {
	    if ((tmpcrc ^ ch) & 1)
		tmpcrc = (tmpcrc >> 1) ^ 0xa001;
	    else
		tmpcrc >>= 1;
	    ch >>= 1;
	}
    }
    return tmpcrc;
}

/*****************************************************************************
 * функция заполнения буфера для передачи через GSM сеть
 ****************************************************************************/
int mkgsm_buf(u8 * out_buf, u8 * in_buf, void *v, short num)
{
    u8 *ptr;
    u16 crc;
    int ind = 0, i;		/* индекс в выходном буфере */
    char nummatts;
    gps_data_t *data = (gps_data_t *) v;

    out_buf[ind++] = 0x23;	//заголовок

    /* номер-идентификатор */
    ptr = (u8 *) & num;
    for (i = 0; i < 2; i++) {
	out_buf[ind++] = *ptr++;
    }

    /* время-дата: 4 байта */
    ptr = (u8 *) & data->time;
    for (i = 0; i < 4; i++) {
	out_buf[ind++] = *ptr++;
    }


    /* широта: 4 байта */
    ptr = (u8 *) & data->lat;
    for (i = 0; i < 4; i++) {
	out_buf[ind++] = *ptr++;
    }

    /* долгота: 4 байта */
    ptr = (u8 *) & data->lon;
    for (i = 0; i < 4; i++) {
	out_buf[ind++] = *ptr++;
    }

    /* горизонтальная скорость - перевести в КМ / Ч: 2 байта */
    ptr = (u8 *) & data->vel;
    for (i = 0; i < 2; i++) {
	out_buf[ind++] = *ptr++;
    }

#if 1
    /* Высота в М: 2 байта */
    ptr = (u8 *) & data->hi;
    for (i = 0; i < 2; i++) {
	out_buf[ind++] = *ptr++;
    }
#endif

    /* число наборов данных (веществ). 14 - Это максимум! */
    nummatts = in_buf[1];	/* 1-й байт в буфере */
    if (nummatts > MAX_NUM_MATS) {
	nummatts = MAX_NUM_MATS;
    }


    out_buf[ind++] = 0x23;
    memcpy(out_buf + ind++, in_buf + 1, nummatts * DATA_SLOT_SIZE + 1);
    ind += nummatts * DATA_SLOT_SIZE;

    /* контрольная сумма */
    crc = mk_crc16(0xFFFF, out_buf, ind);

    /* PRINTF("CRC16=0x%02X\n",crc); */
    ptr = (u8 *) & crc;
    for (i = 0; i < 2; i++) {
	out_buf[ind++] = *ptr++;
    }

    //   char str[128];
    //  memcpy(str, out_buf, ind);

    return ind;			/* размер пакета */
}



/* Найти первое число в строке-источнике между кавычками*/
char *parse_ip_addr(char *src)
{
    int i, p0 = 0, p1 = 0, res = 0;
    int count = 0;
    char *addr = src;

    /* Ищем кавычки */
    for (i = 0; i < strlen(src); i++) {
	if (src[i] == '\"') {
	    count++;
	}

	if (!p0 && count == 1) {
	    p0 = i + 1;
	} else if (!p1 && count == 2) {
	    p1 = i;
	    addr = &src[p0];
	    src[p1] = 0;
	    break;
	}
    }

    return addr;
}



/* Найти первое число в строке-источнике между кавычками*/
char *strnum(char *src)
{
    int i, points = 0, res = 0;
    char *ptr, *addr;

    ptr = src;
    for (i = 0; i < strlen(src); i++) {

	/* Нашли цифру */
	if (isdigit(*ptr) && res == 0) {
	    addr = ptr;
	    res = 1;
	} else if (*ptr == '.') {
	    points++;
	} else if (points >= 3 && (*ptr == '\n' || *ptr == ' ')) {
	    src[i] = 0;
	    break;
	}
	ptr++;
    }

    return addr;
}

/* переопределение printf  */
int PRINTF(char *fmt, ...)
{
    int r = 0;
#if 10
    char str[256];
    va_list p_vargs;		/* return value from vsnprintf  */

    va_start(p_vargs, fmt);
    r = vsnprintf(str, sizeof(str), fmt, p_vargs);
    va_end(p_vargs);

    r = printf(str);
#endif
    return r;
}


/**
 * Разобрать строку от NMEA, чтобы не делать это в ISR
 * $GPRMC,084851.000,A,5546.73904,N,03744.21101,E,0.3,320.5,180615,,,A*6E
 * Пройти в цыкле по буферу и определить позиции времени и координат
 */
void parse_rmc_string(const char *str, void *v)
{
    struct tm t0;
    char buf[16];
    int count = 0;
    gps_data_t *data;
    u8 i;
    f32 fMin;
    int iGrad;


    data = (gps_data_t *) v;
    do {
	u8 status_pos = 0, lon_pos = 0, lat_pos = 0, time_pos = 0, dat_pos = 0, ns_pos = 0;
	u8 we_pos = 0, vel_pos = 0, ang_pos = 0, lat_wid = 0, lon_wid = 0, vel_wid = 0;

	/* $GPRMC,084851.000,A,5546.73904,N,03744.21101,E,0.3,320.5,180615,,,A*6E */
	for (i = 0; i < NMEA_GPRMC_STRING_SIZE; i++) {
	    if (str[i] == 0x2c)
		count++;

	    /* найдем позицию time */
	    if (!time_pos && count == 1) {
		time_pos = i + 1;
	    } else if (!status_pos && count == 2) {
		status_pos = i + 1;	/* найдем позицыю достоверности 'A' */
	    } else if (!lat_pos && count == 3) {
		lat_pos = i + 1;	/* найдем позицыю широты */
	    } else if (!ns_pos && count == 4) {
		ns_pos = i + 1;	/* Позиция полушара N или S */
		lat_wid = ns_pos - lat_pos - 1;
	    } else if (!lon_pos && count == 5) {
		lon_pos = i + 1;	/* найдем позицыю долготы  */
	    } else if (!we_pos && count == 6) {
		we_pos = i + 1;	/* Позиция полушара W или E */
		lon_wid = we_pos - lon_pos - 1;
	    } else if (!vel_pos && count == 7) {	/* Скорость в узлах */
		vel_pos = i + 1;
	    } else if (!ang_pos && count == 8) {	/* Направление движения */
		ang_pos = i + 1;
		vel_wid = ang_pos - vel_pos - 1;
	    } else if (!dat_pos && count == 9) {
		dat_pos = i + 1;	/* Позиция даты */
		break;
	    }
	}

	/* достоверность - не проверяем здесь! */
	//status = (str[status_pos] == 'A') ? true : false;

	/* Время - часы */
	memcpy(buf, str + time_pos, 2);
	buf[2] = 0;
	t0.tm_hour = atoi(buf);

	/* минуты */
	memcpy(buf, str + time_pos + 2, 2);
	buf[2] = 0;
	t0.tm_min = atoi(buf);

	/* секунды */
	memcpy(buf, str + time_pos + 4, 2);
	buf[2] = 0;
	t0.tm_sec = atoi(buf);

	/* Day of the month - [1,31] */
	memcpy(buf, str + dat_pos, 2);
	buf[2] = 0;
	t0.tm_mday = atoi(buf);

	/* Months since January - [0,11] */
	memcpy(buf, str + dat_pos + 2, 2);
	buf[2] = 0;
	t0.tm_mon = atoi(buf) - 1;

	/* Years since 1900 */
	memcpy(buf, str + dat_pos + 4, 2);
	buf[2] = 0;
	t0.tm_year = atoi(buf) + 100;
        data->time = right_mktime(&t0);
        

	/* Широта и долгота занимает */
	if (lat_wid < 2 || lat_wid > 12 || lon_wid < 2 || lon_wid > 12 || vel_wid < 2 || vel_wid > 12) {
	    break;
	}
	// $GPRMC,084851.000,A,5546.73904,N,03744.21101,E,0.3,320.5,180615,,,A*6E
	memcpy(buf, str + lat_pos, lat_wid);
	buf[lat_wid] = 0;
	if (str[ns_pos] == 'N') {
	    data->lat = atof(buf);
	} else if (str[ns_pos] == 'S') {
	    data->lat = -atof(buf);
	} else {
	    break;
	}

	/* Перевести минуты в десятичные доли градуса */
	iGrad = (int) (data->lat / 100.0);	// получаем градусы
	fMin = data->lat - iGrad * 100;	// минуты с дробной частью
	fMin = fMin / 60.0;
	data->lat = (float) iGrad + fMin;

	memcpy(buf, str + lon_pos, lon_wid);
	buf[lon_wid] = 0;
	if (str[we_pos] == 'E') {
	    data->lon = atof(buf);
	} else if (str[we_pos] == 'W') {
	    data->lon = -atof(buf);
	} else {
	    break;
	}

	/* Перевести минуты в десятичные доли градуса */
	iGrad = (int) (data->lon / 100.0);	// получаем градусы
	fMin = data->lon - iGrad * 100;	// минуты с дробной частью
	fMin = fMin / 60.0;
	data->lon = (float) iGrad + fMin;


	/* Скорость */
	memcpy(buf, str + vel_pos, vel_wid);
	buf[vel_wid] = 0;
	data->vel = (int) (atof(buf) * 1.852);

	/* Смотрим время если только если есть статус */
	if (data->lat != 0 && data->lon != 0) {
	    data->time = right_mktime(&t0);
	}

    } while (0);
}


/**
 * Разобрать ответ на AT+CIPGSMLOC=1,1
 */
void parse_loc_string(const char *str, void *v)
{
    struct tm t0;
    int i, res = -1;
    int count = 0;
    char buf[16];
    gps_data_t *data;
    int len = strlen(str);
    if (len > NMEA_GPRMC_STRING_SIZE)
	len = NMEA_GPRMC_STRING_SIZE;

    data = (gps_data_t *) v;

    do {
	u8 zero_pos = 0, lat_pos = 0, lon_pos = 0, tim_pos = 0, dat_pos = 0;
	u8 lat_wid = 0, lon_wid = 0, dat_wid = 0, tim_wid = 0;

	/* Найдем +CIPGSMLOC */
	if (strstr(str, "+CIPGSMLOC:") == NULL) {
	    break;
	}

	for (i = 1; i < len - 1; i++) {
	    if (str[i] == 0x2c)
		count++;
	    else if (str[i] == '\n' && count == 4)
		count = 5;



	    if (!lon_pos && count == 1 && str[i - 1] == '0') {
		lon_pos = i + 1;	/* найдем позицыю долготы */
		zero_pos = i - 1;
	    } else if (!lat_pos && count == 2) {
		lat_pos = i + 1;	/* Позиция широты */
		lon_wid = lat_pos - zero_pos;
	    } else if (!dat_pos && count == 3) {
		dat_pos = i + 1;	/* найдем позицыю даты  */
		lat_wid = dat_pos - lat_pos - 1;
	    } else if (!tim_pos && count == 4) {
		tim_pos = i + 1;	/* Позиция времени */
		dat_wid = tim_pos - dat_pos - 1;
	    } else if (count == 5) {
		tim_wid = i - tim_pos;	/* ширина времени */
		break;
	    }
	}
	// ошибка
	if (count < 5) {
	    break;
	}
//+CIPGSMLOC: 0,37.741424,55.779529,2015/12/07,09:02:54 
	/* Широта и долгота занимает */
	if (lat_wid < 2 || lat_wid > 16 || lon_wid < 2 || lon_wid > 16 || dat_wid < 8 || dat_wid > 12 || tim_wid < 8 || tim_wid > 10) {
	    break;
	}

	/* Время - часы */
	memcpy(buf, str + tim_pos, 2);
	buf[2] = 0;
	t0.tm_hour = atoi(buf);

	/* минуты */
	memcpy(buf, str + tim_pos + 3, 2);
	buf[2] = 0;
	t0.tm_min = atoi(buf);

	/* секунды */
	memcpy(buf, str + tim_pos + 6, 2);
	buf[2] = 0;
	t0.tm_sec = atoi(buf);


	/* Years */
	memcpy(buf, str + dat_pos, 4);
	buf[4] = 0;
	t0.tm_year = atoi(buf) - 1900;

	/* Months since January - [0,11] */
	memcpy(buf, str + dat_pos + 5, 2);
	buf[2] = 0;
	t0.tm_mon = atoi(buf) - 1;

	/* Day of the month - [1,31] */
	memcpy(buf, str + dat_pos + 8, 2);
	buf[2] = 0;
	t0.tm_mday = atoi(buf);

	/* Смотрим время */
	data->time = right_mktime(&t0);


//+CIPGSMLOC: 0,37.741424,55.779529,2015/12/07,09:02:54

	/* Перевести минуты в десятичные доли градуса */
	memcpy(buf, str + lat_pos, lat_wid);
	buf[lat_wid] = 0;
	data->lat = atof(buf);

	/* Перевести минуты в десятичные доли градуса */
	memcpy(buf, str + lon_pos, lon_wid);
	buf[lon_wid] = 0;
	data->lon = atof(buf);
	res = 0;

	data->vel = data->hi = 0;
	data->status = FIX_GSM;
    }
    while (0);
}

/**
 * Разобрать строку от NMEA, чтобы не делать это в ISR
 * $GPGGA,105557.000,5546.72977,N,03744.22356,E,1,04,5.2,0172.3,M,14.4,M,,*60
 * Пройти в цыкле по буферу и определить позиции времени и координат
 */
void parse_gga_string(const char *str, void *v)
{
    int count = 0;
    gps_data_t *data;
    u8 i, len;
    char buf[16];

    len = strlen(str);
    if (len > NMEA_GPRMC_STRING_SIZE)
	len = NMEA_GPRMC_STRING_SIZE;

    data = (gps_data_t *) v;

    do {
	u8 hi_pos = 0, hi_wid = 0, m_pos = 0, fix_pos = 0;

	// Ищем 9-ю запятую
	for (i = 0; i < len; i++) {
	    if (str[i] == 0x2c)
		count++;


	    if (!fix_pos && count == 6) {
		fix_pos = i + 1;	/* Фикс */
	    } else if (!hi_pos && count == 9) {
		hi_pos = i + 1;	/* Высота */
	    } else if (!m_pos && count == 10) {
		m_pos = i + 1;
		hi_wid = m_pos - hi_pos - 1;
		break;
	    }
	}

	if (hi_wid < 3)
	    break;

	memcpy(buf, str + hi_pos, hi_wid);
	buf[hi_wid] = 0;
	if (str[m_pos] == 'M' || str[m_pos] == 'm') {
	    data->hi = (s16) atof(buf);
	}

	/* 3D fix или какой? Ищем 6-ю запятую */
	memcpy(buf, str + fix_pos, 1);
	buf[fix_pos + 1] = 0;
	i = atoi(str + fix_pos);
	if (i >= 0) {
	    data->status = FIX_3D;
	}
    } while (0);
    /* PRINTF("%s\n", str); */
}


/*************************************************************
 * Вывод данных в HEX на экран
 *************************************************************/
void print_data_hex(void * data, int len)
{
    int i;
    for (i = 0; i < len; i++) {
	if (i % 8 == 0 && i != 0) {
	    printf(" ");
	}
	if (i % 16 == 0 && i != 0 && i != 8) {
	    printf("\n");
	}
	printf("%02X ", *(((u8 *) data) + i));
    }
    printf("\n");
}



static struct {
    unsigned long num;		// номер измерения
    long sum_data;		// сумма элементов
    long sum_quad;		// сумма квадратов
    f32 variance;
} disp_data;

/**
 * Считать дисперсию
 */
f32 get_variance(int data)
{
    /* копим среднее */
    disp_data.sum_data += (long) data;
    disp_data.sum_quad += (long) data *data;
    disp_data.num++;

    if (disp_data.num == 0) {
	disp_data.num = 1;
    }
    disp_data.variance = disp_data.sum_quad / (disp_data.num) - disp_data.sum_data * disp_data.sum_data / disp_data.num / disp_data.num;
    return disp_data.variance;
}
