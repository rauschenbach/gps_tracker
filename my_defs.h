#ifndef _MY_DEFS_H
#define _MY_DEFS_H

#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <float.h>

#ifndef u8
#define u8 unsigned char
#endif

#ifndef s8
#define s8 char
#endif

#ifndef c8
#define c8 char
#endif

#ifndef u16
#define u16 unsigned short
#endif


#ifndef s16
#define s16 short
#endif

#ifndef i32
#define i32  int32_t
#endif


#ifndef u32
#define u32 uint32_t
#endif


#ifndef s32
#define s32 int32_t
#endif


#ifndef u64
#define u64 uint64_t
#endif


#ifndef s64
#define s64 int64_t
#endif


/* Длинное время */
#ifndef	time64
#define time64	int64_t
#endif


/* long double не поддержываеца -  в линуксе может такого не быть! */
#ifndef f32
#define f32 float
#endif


/*
typedef char c8;
typedef unsigned char u8;
typedef int i32;
typedef uint64_t u64;
typedef int16_t  s16;
typedef uint16_t  u16;
typedef int64_t  s64;
typedef float    f32;
typedef unsigned char bool;
typedef signed int s32;
typedef unsigned int u32;

typedef char c8;
typedef int i32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef float    f32;
typedef unsigned char bool;

*/
#ifndef bool
#define bool u8
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif


#ifndef IDEF
#define IDEF static inline
#endif


#define 	M_PI		3.141592653589


/* Данные акселерометра или данные компаса */
typedef struct {
	f32 x;
#define pitch 	x
	f32 y;
#define roll	y	
	f32 z;
#define head	z
	f32 t;			// + температура или коэффициент для нормализации
#define norm 	t
} acc_data;


/*
typedef struct {
	int x;
	int y;
	int z;
	int t;			// + температура
} acc_data;
*/


typedef struct {
	u16  len;    	/* FF00 */
	u16  rsvd0;	 
	u32  unix_time;
	f32  norm_div; 
	acc_data acc0;	/* ускорения */
	acc_data mag;	/* напр. поля */
	acc_data acc1;	/* ускорения */
	acc_data ang; /* углы наклона и поворота */
	acc_data vel;   /* скорости */
	acc_data path;	/* расстояния */
	u16  rsvd1;
	u16  crc16;
} send_pack_data;



#endif				/* my_defs.h */
