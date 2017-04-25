#ifndef _SIM900_H
#define _SIM900_H

#include "main.h"

typedef enum {
  GSM_OK = 0,
  GSM_WORK = 1,
  GSM_TIMEOUT,
  GSM_CLOSED,
  GSM_ERROR,
  GSM_FAIL,
  GSM_DEACT, 
  GSM_NO_ADDR,
  GSM_POWER_DOWN, 
} GSM_ENUM;


int sim900_power_on(void);
int sim900_init(void);
int sim900_gsm_on(void);
int sim900_gsm_off(void);
int sim900_wait_connect(void);
int sim900_get_ip_addr(void);
int sim900_get_coord(gps_data_t*);
int sim900_tcp_connect(void);
int sim900_tcp_close(void);
int sim900_send_data(u8 *, u16);
int sim900_gsm_check(void);
int sim900_gsm_status(void);

#endif /* SIM900 */
