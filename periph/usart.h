#ifndef _USART_H
#define _USART_H

#include <stdio.h>
#include "main.h"


/* Максимальное значение строки 128 символов */
#define 	USART_BUF_SIZE 			88
#define 	USART_INFO_SIZE			128
#define 	USART_CHECK_SYMBOL 		0xA5


int  fputc(int ch, FILE *f);
void USART1_IRQHandler(void);
int usart_task_create(int, int, int);
void usart_buf_parse(u8);
void usart_get_gank_buf(u8*, int);
bool usart_check_gank_buf(void);
void usart_irq_enable(void);
void usart_irq_disable(void);


#endif /* uart.h */
