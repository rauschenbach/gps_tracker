#ifndef _GSM_H
#define _GSM_H

#include "main.h"
#include "gps.h"

/* Максимальное значение строки для передачи и приема */
#define 	GSM_TX_STRING_SIZE 	(128)
#define 	GSM_RX_STRING_SIZE 	(128)

#define 	GSM_USART		UART4
#define 	PWRKEY			GPIO_Pin_4	/* PB4 на порту B  */
#define 	GSMON			GPIO_Pin_9	/* PB9 на порту B  */

/* Сообщение на отправку */
typedef struct GSM_INFO_MSG{
      u8  info_buf[GSM_TX_STRING_SIZE];
      int info_len; 
} gsm_info_msg_struct;


/**
 * Состояние КА 
 */
typedef enum {
   GSM_POWER_ON_STATE,
   GSM_INIT_STATE,
   GSM_NET_ON_STATE,
   GSM_CONNECT_STATE,
   GSM_SEND_DATA_STATE,    
   GSM_ERROR_STATE,    
} GSM_STATE_t;


typedef enum {
   GSM_BUF_IN,
   GSM_BUF_OUT
} buf_type_t;


int  gsm_task_create(int, int, int);
int  gsm_send_buf(u8*, int);
int  gsm_send_str(char *);
int  gsm_get_str(char *, int);
void gsm_read_byte_isr(u8);
void gsm_write_byte_isr(void);
void gsm_sim900_on_off(void);
int  gsm_wait_for_str(const char*, int);
bool gsm_wait_for_ip_addr(char *, int);
void gsm_flush(buf_type_t);
bool gsm_wait_for_gsm_coord(gps_data_t*, int);
void* gsm_get_sem_ptr(void);
void USART2_IRQHandler(void);


#endif /*  _GSM_H */
