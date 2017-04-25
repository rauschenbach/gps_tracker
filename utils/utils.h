#ifndef _UTILS_H
#define _UTILS_H

#include "main.h"


/* Ставим void и приводим тип в теле функцыи - иначе компилятор пишет ошибку */
void parse_rmc_string(const char *, void *);
void parse_gga_string(const char *, void *);
int  mkgsm_buf(u8 *, u8 *, /*gps_data_t*/void*, short);
long mk_crc16(long, void *, u32);
void print_nmea_data(void *);
char *strnum(char *);
void parse_loc_string(const char *, void *);
char *parse_ip_addr(char *src);
int PRINTF(char*,...);
void time_to_str(char *);
void print_data_hex(void*, int);
int  sec_to_tm(long, struct tm *);
int  sec_to_str(long, char *);
long tm_to_sec(struct tm *);


#endif				/* utils.h */
