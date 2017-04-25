/*****************************************************************************
 *  Здесь запускается 3 usart на прием и парсится строка $GPRMC 
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "gps.h"

#define 	GPS_USART	USART3
#define		GPS_TX_PIN	GPIO_Pin_10
#define		GPS_RX_PIN	GPIO_Pin_11

#define 	FORCE_ON_PIN 	GPIO_Pin_5 /* PA5 */
#define 	GPS_RESET_PIN	GPIO_Pin_6 /* PA6  */
#define 	GPS_ON_PIN 	GPIO_Pin_7 /* PA7 */			


static xQueueHandle gps_rx_queue;	/* The queue used to send messages to the task. */
static xSemaphoreHandle gps_rx_sem;	/* Семафор для передачи управления задаче */
static gps_data_t gps_data;


static void vGpsTask(void *pvParameters);


/**
  * GPS_USART для приема строки $GPRMC
  * Используем только на прием - передача нам не нужна
  */
int gps_task_create(int baud, int stack, int prio)
{
    int res = -1;
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    do {

	/* Создаем семафор */
	vSemaphoreCreateBinary(gps_rx_sem);

	/* Create the queue на приём - 128 значений */
	gps_rx_queue = xQueueCreate(NMEA_GPRMC_STRING_SIZE, sizeof(u8));
	if (gps_rx_queue != INVALID_QUEUE) {

	    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);


	    /*  GPS_USART Tx (PB.10)  */
	    GPIO_InitStructure.GPIO_Pin = GPS_TX_PIN;
	    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	    GPIO_Init(GPIOB, &GPIO_InitStructure);

	    /*  GPS_USART Rx (PB.11)  */
	    GPIO_InitStructure.GPIO_Pin = GPS_RX_PIN;
	    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	    GPIO_Init(GPIOB, &GPIO_InitStructure);

	    USART_InitStructure.USART_BaudRate = baud;	// 115200; //bps
	    USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//
	    USART_InitStructure.USART_StopBits = USART_StopBits_1;	//
	    USART_InitStructure.USART_Parity = USART_Parity_No;	//
	    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//
	    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	    USART_Init(GPS_USART, &USART_InitStructure);
	    USART_Cmd(GPS_USART, ENABLE);

	    /* Только прием! */
	    USART_ITConfig(GPS_USART, USART_IT_RXNE, ENABLE);

	    /* Это прерывание уже было разрешено в board.c ? */
	    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = configLIBRARY_KERNEL_INTERRUPT_PRIORITY;
	    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	    NVIC_Init(&NVIC_InitStructure);

	    /* Создадим задачу для разбора данных */
	    xTaskCreate(vGpsTask, "GPS", stack, NULL, prio, NULL);

	    /* Это прерывание уже было разрешено в board.c */
	    NVIC_EnableIRQ(USART3_IRQn);
            
	    res = 0;
	}
    } while (0);
    return res;
}



/** 
 * Задача разбирающая данные - Прием по UART0 
 * Ждем прихода сигнакла на разбор
 * Минимальный размер стека 512!
 */
static void vGpsTask(void *par)
{
    u8 rx_buf[NMEA_GPRMC_STRING_SIZE];	/* Суда сливаем для разбора */
    u8 rx_byte;
    u8 rx_beg;
    u8 rx_cnt;
    
    gps_on();

    while (1) {

	/* Ждем семафора */
	if (xSemaphoreTake(gps_rx_sem, TIME_DELAY_5_MS)) {

	    /* Берем даные из очереди  */
	    while (xQueueReceive(gps_rx_queue, &rx_byte, portMAX_DELAY) == pdPASS) {
		if (rx_byte == '$') {	/*  Первый байт.  Начинается всегда с $ */
		    rx_beg = 1;
		    rx_cnt = 1;
		    rx_buf[0] = rx_byte;
		} else if (rx_beg == 1) {	// Прием уже начался
		    /* Сливаем в другой буфер, пока не знаю - хватит ли времени для разбора */
		    rx_buf[rx_cnt] = rx_byte;
                    
                    
/*                
$GPRMC,084659.000,A,5546.75545,N,03744.19997,E,0.9,356.3,250316,,,A*6E
$GPGGA,084659.000,5546.75545,N,03744.19997,E,1,05,3.8,01,0.9,N,1.6,K,A*00
$GNGSA,A,3,30,07,23,09,27,,,,,,,,4.2,3.8,1.9*20
$GPGSV,3,1,11,02,14,273,,05,27,311,,06,02,233,,07,69,221,35*70
$GPGSV,3,2,11,08,00,111,,09,68,121,30,16,31,048,,23,35,113,3231,34,,,,*45
$GPGLL,5546.75545,N,03744.19997,E,084659.000,A,A*50
  */

		    /*  Проверка, перевод строки/ Собираем пакеты RMC и GSA и PMTK */
		    if (rx_byte == '\x0A') {
			rx_buf[rx_cnt] = 0;
			rx_beg = 0;


			/*  $GPRMC,134454.000,A,5546.70518,N,03744.20053,E,0.7,216.7,170615,,,A*65 */
			if (rx_buf[3] == 'R' && rx_buf[4] == 'M' && rx_buf[5] == 'C') {
			    parse_rmc_string((c8 *) rx_buf, &gps_data);
		   	   // PRINTF("%s", rx_buf);

			    /*   $GPGGA,084659.000,5546.75545,N,03744.19997,E,1,05,3.8,01,0.9,N,1.6,K,A*00 
			       0 = Определение местоположения не возможно или не верно;
			       1 = GPS режим обычной точности, возможно определение местоположения;
			       2 = Дифференциальный GPS режим, точность обычная, возможно определение местоположения;
			       3 = GPS режим прецизионной точности, возможно определение местоположения;
			       4 = RTK fixed (фиксированное решение Позиционирования в Реальном Времени);
			       5 = RTK float (плавающее решение Позиционирования в Реальном Времени).                        
			     */
			} else if (rx_buf[3] == 'G' && rx_buf[4] == 'G' && rx_buf[5] == 'A') {
			    led_toggle(LED2);
			    parse_gga_string((c8 *) rx_buf, &gps_data);
			}

			break;
		    } else {
			rx_cnt++;
			if (rx_cnt >= NMEA_GPRMC_STRING_SIZE) {
			    rx_beg = rx_cnt = 0;
			}
		    }
		}
	    }
	}
	vTaskDelay(TIME_DELAY_5_MS);
    }
}

/* Получить статус NMEA */
u8 gps_get_status(void)
{
    return gps_data.status;
}

/* Получить данные NMEA */
void gps_get_data(gps_data_t * d)
{
    memcpy(d, &gps_data, sizeof(gps_data_t));
}


/* Получить данные NMEA */
void gps_set_data(gps_data_t * d)
{
    memcpy(&gps_data, d, sizeof(gps_data_t));
}

/* Включить GPS модуль */
void gps_on(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
 
    /* Разрешим тактирование PA */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Включаем питание на FORCE_ON */
    GPIO_InitStructure.GPIO_Pin = FORCE_ON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, FORCE_ON_PIN);
    vTaskDelay(1000);  
    GPIO_ResetBits(GPIOA, FORCE_ON_PIN);

    /* Убираем 0 с Reset  */
    GPIO_InitStructure.GPIO_Pin = GPS_RESET_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOA, FORCE_ON_PIN);


    /* Питание */
    GPIO_InitStructure.GPIO_Pin = GPS_ON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPS_ON_PIN);

    vTaskDelay(100);  
}


/**
  * @brief  Принимаем байты в в буфер в этой функции до перевода строки
  * И пересылаем  в очередь
  */
void USART3_IRQHandler(void)
{
    char byte;
    volatile u32 status;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    status = GPS_USART->SR;


    /* Читаем байт и заполняем очередь по приему */
    if (status & USART_FLAG_RXNE) {
	byte = USART_ReceiveData(GPS_USART);

	/* Пишем в очередь принятый байт */
	xQueueSendFromISR(gps_rx_queue, &byte, &xHigherPriorityTaskWoken);

	/* По приему перевода строки - посылаем сигнал */
	if (byte == '\n' && xSemaphoreGiveFromISR(gps_rx_sem, &xHigherPriorityTaskWoken)) {
	    /* Если входим в прерывание - передадим управление */
	    if (xHigherPriorityTaskWoken) {
		taskYIELD();
	    }
	}
    }
    /* OverRun + Noise + Framing + Parity Error */
    if (status & 0x0F) {
	byte = GPS_USART->DR;
    }
//  NVIC_ClearPendingIRQ(USART3_IRQn);    
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
