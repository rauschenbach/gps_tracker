#ifndef _GPS_H
#define _GPS_H

#include "my_defs.h"



/* Максимальное значение строки GPRMC > 82 символа */
#define 	NMEA_GPRMC_STRING_SIZE 			128
#define 	RX_FIN_SIGN 				0x5A


typedef enum {
  FIX_VOID,
  FIX_GSM,
  FIX_2D,
  FIX_3D
} gps_fix_t;


typedef struct {
  s32  time; /* Время по UTC */
  f32  lat;	/* Широта  */
  f32  lon;     /* Долгота */
  s16  vel;     /* Скорость */
  s16  hi;	/* Высота до 1000 м */
  u8   status;	/* Status (new): 0 - нет фикса, 1 - данные от GSM, 2 - 2 Dfix, 3- 3dfix */
  u8   rsvd;
} gps_data_t;



int  gps_task_create(int, int, int);
void gps_nmea_parse(char);
int  gps_get_str(char *, int);
u8   gps_get_status(void);
void gps_get_data(gps_data_t*);
void gps_set_data(gps_data_t *);
void vGPSInterruptHandler(void);
void *get_queue_ptr(void);
void gps_on(void);
void USART3_IRQHandler(void);

#endif /* gps.h */
