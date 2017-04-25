#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "main.h"
#include "gsm.h"

#define		COM_USART	USART1
#define		COM_TX_PIN	GPIO_Pin_9
#define		COM_RX_PIN	GPIO_Pin_10




static void usart_reset_buf(void);
static void vUsartSendTask(void *p);
static xSemaphoreHandle usart_tx_sem;	/* Семафор для передачи управления задаче */


/* Прием по UART1 - выделяем память на указатель на нашу структуру */
static struct USART_DATA_STRUCT {
    u8 rx_buf[USART_BUF_SIZE];	/* Суда сливаем для разбора */
    u8 rx_beg;
    u8 rx_cnt;
    u8 rx_len;
    u8 rx_check;		/* можно забирать */
} usart_data;



/**
  * @brief  COM_USART configuration 
  * @param  None
  * @retval None
  */
int usart_task_create(int baud, int stack, int prio)
{
    int res = -1;

    /* Создаем семафор */
    vSemaphoreCreateBinary(usart_tx_sem);
    memset(&usart_data, 0, sizeof(usart_data));

    do {
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable COM_USART clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure COM_USART Rx (PA10) as input floating */
	GPIO_InitStructure.GPIO_Pin = COM_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure COM_USART Tx (PA9) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = COM_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(COM_USART, &USART_InitStructure);

	USART_ITConfig(COM_USART, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


//	xTaskCreate(vUsartSendTask, "USART_Send_task", stack, NULL, prio, NULL);
	USART_Cmd(COM_USART, ENABLE);
	res = 0;
    } while (0);
    return res;
}

/**
 * Формируем сообщение на отсылку, в зависимости от числа слотов и достоверности GPS
 */
static void vUsartSendTask(void *p)
{
    gps_data_t gps;		/* Данные GPS */
    int i;
    xSemaphoreHandle *pSem;
    gsm_info_msg_struct* msg;
    
    msg = (gsm_info_msg_struct*)gsm_get_msg_ptr();

    while (1) {

	/* Ждем семафора по готовности буфера и передаем данные */
	if (xSemaphoreTake(usart_tx_sem, 1) && USART_CHECK_SYMBOL == usart_data.rx_check) {	  
          
            /* Получим кординаты - другая задача следит за их достоверностью */
	    gps_get_data(&gps);
            
            /* Будем изменять вещества для теста */
            for(i = 0; i < usart_data.rx_buf[1]; i++) {
              float ds;
              int   tmp = rand() % 256;
              int  sign = (rand() % 2 == 1)?  -1 : 1;
              memcpy(&ds, &usart_data.rx_buf[4 + i * 6], sizeof(float));
              ds = ds + ((float)tmp / 1000.0) * sign;
              memcpy(&usart_data.rx_buf[4 + i * 6], &ds, sizeof(float));
            }
            

	    /* Делаем буфер на передачу */
	    msg->info_len = mkgsm_buf(msg->info_buf, usart_data.rx_buf, &gps, SERIAL_NUM);

            pSem = gsm_get_sem_ptr();

            /* Отдать семафор  */
            xSemaphoreGive(*(xSemaphoreHandle*)pSem);


	    led_toggle(LED3);
	}
	vTaskDelay(10);
    }
}



/**
 * Включить прерывание на прием, чтобы можно было принимать команды
 */
void usart_irq_enable(void)
{
    memset(&usart_data, 0, sizeof(usart_data));

    USART_ITConfig(COM_USART, USART_IT_RXNE, ENABLE);
}


/**
 * Выключить прерывание, чтобы можно было видеть отладкку
 */
void usart_irq_disable(void)
{
    USART_ITConfig(COM_USART, USART_IT_RXNE, DISABLE);
}


/* Сбросить буфер */
static void usart_reset_buf(void)
{
    usart_data.rx_beg = 0;
    usart_data.rx_cnt = 0;
}

/* e.g. write a character to the USART */
int fputc(int ch, FILE * f)
{
    USART_SendData(COM_USART, (uint8_t) ch);

    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(COM_USART, USART_FLAG_TC) == RESET) {    }
    return ch;
}


/*  Приходит из прерывания! - собираем буфер */
void usart_buf_parse(u8 rx_byte)
{
    if (usart_data.rx_cnt >= USART_BUF_SIZE) {
	usart_reset_buf();
    }

    /*  Первый байт */
    if (usart_data.rx_beg == 0 && rx_byte == 0x23) {	/* Начинается всегда с # */
	usart_data.rx_check = 0;	/* Сбрасываем */
	usart_data.rx_beg = 1;
	usart_data.rx_cnt = 1;
        usart_data.rx_buf[0] = rx_byte;        
	/*Включаем таймаут здесь! */
    } else if (usart_data.rx_beg == 1 && usart_data.rx_cnt == 1) {	// Прием уже начался
	usart_data.rx_len = rx_byte;
	usart_data.rx_cnt = 2;

	if (usart_data.rx_len > 14) {
	    usart_reset_buf();
	}
        usart_data.rx_len *= 6;
        usart_data.rx_buf[1] = rx_byte;
    } else if (usart_data.rx_beg == 1 && usart_data.rx_cnt < usart_data.rx_len + 3) {
        usart_data.rx_buf[usart_data.rx_cnt] = rx_byte;
	usart_data.rx_cnt++;
  
        /* Последний символ */
        if(usart_data.rx_cnt == usart_data.rx_len + 2) {
            signed portBASE_TYPE flag = false;
            usart_data.rx_check = USART_CHECK_SYMBOL;
            xSemaphoreGiveFromISR(usart_tx_sem, &flag);
            usart_reset_buf();
        }      
    }
}

/**
  * @brief  This function handles COM_USART global interrupt request.
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
    volatile u8 byte;
    volatile u32 status;

    status = COM_USART->SR;

    /* Read one byte from the receive data register */
    if (status & USART_FLAG_RXNE) {
	byte = USART_ReceiveData(COM_USART);
	usart_buf_parse(byte);
    }

    /* Write one byte to the transmit data register */
    if (status & USART_FLAG_TXE) {
	// USART_ITConfig(COM_USART, USART_IT_TXE, DISABLE); 
    }

    /* OverRun + Noise + Framing + Parity Error */
    if (status & 0x0F) {
	//USART3->SR |= 0xf;
	byte = COM_USART->DR;
	//    NVIC_ClearPendingIRQ(USART1_IRQn);            
    }
}


/* Получить буфер с данными. Сбросить достоверность */
void usart_get_gank_buf(u8 * buf, int len)
{
    int n = (len > USART_BUF_SIZE) ? USART_BUF_SIZE : len;
    memcpy(buf, usart_data.rx_buf, n);
    usart_data.rx_check = 0;
}

/* Проверить достоверность */
bool usart_check_gank_buf(void)
{
    bool res = false;

    if (usart_data.rx_check == USART_CHECK_SYMBOL) {
	usart_data.rx_check = 0;
	res = true;
    }

    return res;
}

