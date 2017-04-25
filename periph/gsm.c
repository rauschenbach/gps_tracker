/*****************************************************************************
 * ����� ����������� UART4 �� �����/�������� ��� ���������� SIM9000
 * ��� ��� ���������� WIFI. C ��������� FreeRTOS
 ****************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "circbuf.h"
#include "sim900.h"
#include "usart.h"
#include "gsm.h"


#define                 TASK_DELAY_TIME                 250

/* ������������ ����������*/
static GSM_STATE_t dev_gsm_state;
static xQueueHandle rx_queue;	/* �������� ����� �� ����� */
static xQueueHandle tx_queue;	/* �������� ����� �� �������� */
static gsm_info_msg_struct gsm_info_msg;/* ��������� �� �������� */



/* ��������� �� ������� ������ */
static struct {
    c8 buf[GSM_RX_STRING_SIZE];
    int num;
} rx_struct;

static xSemaphoreHandle gsm_rx_sem;	/* ������� ��� �������� ���������� ������ */
static xSemaphoreHandle gsm_tx_sem;	/* ������� ��� �������� ������ */
static gps_data_t gsm_loc;

/* ��������� �� �������, ����� �� �������  extern */
void* gsm_get_sem_ptr(void)
{
  return &gsm_tx_sem;
}

/* ��������� �� ��������� �������� */
void* gsm_get_msg_ptr(void)
{
   return &gsm_info_msg;
}



/* ����������� ������� */
static void vGsmTask(void *);
static void vTestTask(void *);
static int OnGsmPowerOnState(void);
static int OnGsmInitState(void);
static int OnGsmNetOnState(void);
static int OnGsmConnectState(void);
static int OnGsmSendDataState(void);

/**
  * USART �� ����� � ��������
  */
int gsm_task_create(int baud, int stack, int prio)
{
    int res = -1;

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* ������� ������� */
    vSemaphoreCreateBinary(gsm_rx_sem);

    /* ������� ����� �� ����� */
    rx_queue = xQueueCreate(GSM_RX_STRING_SIZE, sizeof(u8));

    /* ������� ������� �� �������� */
    tx_queue = xQueueCreate(GSM_TX_STRING_SIZE, sizeof(u8));

    if (rx_queue != INVALID_QUEUE && tx_queue != INVALID_QUEUE) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);        

	/*  UART4_TX -> PC10 , UART4_RX -> PC11 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
       

	USART_InitStructure.USART_BaudRate = baud;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(GSM_USART, &USART_InitStructure);
	USART_Cmd(GSM_USART, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_EnableIRQ(UART4_IRQn);

	/* �������� ������ ��� ������� ������ */
//	xTaskCreate(vGsmTask, "GSM", stack, NULL, prio, NULL);
	xTaskCreate(vTestTask, "GSM", stack, NULL, prio, NULL);        

	/* ��� ��������� */
	USART_ITConfig(GSM_USART, USART_IT_RXNE, ENABLE);	/* �������� ���������� */

	res = 0;
    }
    return res;
}

/**
 * ������ ����������� ������ � ���������� ������ � ��������� 
 */
static void vTestTask(void *par)
{
    u8 byte;

    gsm_sim900_on_off();
    rx_struct.num = 0;

    while (1) {

	/* ���� �������� �� �������� ������ */
	if (xSemaphoreTake(gsm_rx_sem, 1)) {

	    rx_struct.num = 0;

	    /* ����� ����� �� ������� - ���������� � ������ */
	    while (xQueueReceive(rx_queue, &byte, portMAX_DELAY) == pdPASS) {
		rx_struct.buf[rx_struct.num++ % GSM_RX_STRING_SIZE] = byte;
		rx_struct.buf[rx_struct.num] = 0;	/* ����� ������� */
	    }
	    /*  �� ������  */
	    if (rx_struct.num) {
		printf("%s", rx_struct.buf);
	    }
	}
	vTaskDelay(1);
    }
}


/**
 * ������ ����������� ������ � ���������� ������ � ��������� 
 */
static void vGsmTask(void *par)
{
    int res;
    dev_gsm_state = GSM_POWER_ON_STATE;	/* ��������� ������� */
    
    printf(" GSM Init\r\n");


    while (1) {

	switch (dev_gsm_state) {

	    /* ���������� */
	case GSM_POWER_ON_STATE:
	    res = OnGsmPowerOnState();
	    if (GSM_OK == res) {
		dev_gsm_state = GSM_INIT_STATE;	/* �������� ��������� � ������ ������ */
		printf("SIM900 Power On OK\r\n");
	    } else if (GSM_POWER_DOWN == res) {
		dev_gsm_state = GSM_POWER_ON_STATE;	/* ���������� - Power down. ������ ����� */
		printf("Power OFF\r\n");
	    } else if (GSM_TIMEOUT == res) {
		dev_gsm_state = GSM_POWER_ON_STATE;	/* ������� - ������� ����� */
		printf("Timeout. Try again...\r\n");
	    } else {
		dev_gsm_state = GSM_ERROR_STATE;
		printf("Error\r\n");
	    }
	    break;

	    /* �������������. ����������� ���. ������ �������� �������. ������� ���� � ��... */
	case GSM_INIT_STATE:
	    res = OnGsmInitState();
	    if (GSM_OK == res) {
		dev_gsm_state = GSM_NET_ON_STATE;
		printf("SIM900 Init OK\r\n");
	    } else if (GSM_TIMEOUT == res) {
		dev_gsm_state = GSM_INIT_STATE;	/* ������ ����� */
		printf("Timeout. Try again...\r\n");
	    } else {
		dev_gsm_state = GSM_ERROR_STATE;
		printf("Error\r\n");
	    }
	    break;

	    /*  ����������� � ���� 1 ���. */
	case GSM_NET_ON_STATE:
	    res = OnGsmNetOnState();
	    if (GSM_OK == res) {
		dev_gsm_state = GSM_CONNECT_STATE;
		printf("SIM900 Network On OK\r\n");
	    } else if (GSM_TIMEOUT == res) {
		dev_gsm_state = GSM_NET_ON_STATE;	/* ������ ����� */
		printf("Timeout. Try again...\r\n");
	    } else {
		dev_gsm_state = GSM_ERROR_STATE;
		printf("Error\r\n");
	    }
	    break;

	    /*  ����������� � ����. ������ ���. �������� �����������. �������� IP ����� � ���������� */
	case GSM_CONNECT_STATE:
	    res = OnGsmConnectState();
	    if (GSM_OK == res) {
		dev_gsm_state = GSM_SEND_DATA_STATE;	/* �������� ��� */
		printf("SIM900 Connect GPRS OK\r\n");
	    } else if (GSM_TIMEOUT == res) {
		dev_gsm_state = GSM_CONNECT_STATE;	/* ������ ����� */
		printf("Timeout. Try again...\r\n");
	    } else {
		dev_gsm_state = GSM_ERROR_STATE;
		printf("Error\r\n");
	    }
	    break;

	    /* ������� ������. � ���� ������ ���� �������� �� ����. ����� � �������� �����  */
	case GSM_SEND_DATA_STATE:
	    res = OnGsmSendDataState();
	    if (GSM_OK == res) {
		dev_gsm_state = GSM_SEND_DATA_STATE;
	    } else if (GSM_CLOSED == res) {
		printf("CLOSED \r\n");
	    } else if (GSM_FAIL == res) {
		printf("FAIL\r\n");
	    } else if (GSM_DEACT == res) {
		printf("DEACT\r\n");
	    } else if (GSM_ERROR == res) {
		dev_gsm_state = GSM_ERROR_STATE;
		printf("Error\r\n");
	    }
	    break;
            
        case GSM_ERROR_STATE:
            vTaskDelay(250);
            dev_gsm_state = GSM_POWER_ON_STATE;
          break;
          

	default:
	    break;
	}

	vTaskDelay(500);
    }
}


/* ����� ����� ������ �� ���������� ������ - �� �������� ������ ��� ���� */
int gsm_get_str(char *str, int len)
{
    int i = 0;
    c8 byte;

    /* ������ �� ���������� ������ */
    while (xQueueReceive(rx_queue, &byte, 5) == pdPASS) {
	/* ������� ������� ������ � ������ ������ */
	if (i < 2 && (byte == '\r' || byte == '\n')) {
	    continue;
	}

	str[i] = byte;
	i++;
	if (byte == '\r' || byte == '\n' || byte == 0 || i > len) {
	    break;
	}
    }

    str[i] = 0;			/* ������� ������. ����� ����������� !!!!!!!  */
    return i;
}


/* �������� ������ � ������ ������ ���� ��� �������� 
 * ������� ��� ������ 0
 */
int gsm_wait_for_str(const char *str, int time_ms)
{
    char *ptr0, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    char *s = rx_struct.buf;	/* ��������� �� ����� */
    int res = GSM_TIMEOUT;
    int n;

    set_timeout(time_ms);
    while (!is_timeout()) {

	n = gsm_get_str(s, 12);


	/* ���������� ��� � ������  �� ������ ������� */
	if (n) {
	    /*  ����� ��������� ��� ��������� ������ */
	    printf("%s ", s);
	    ptr5 = strstr(s, "DOWN");
	    ptr4 = strstr(s, "CLOSED");
	    ptr3 = strstr(s, "DEACT");
	    ptr2 = strstr(s, "FAIL");
	    ptr1 = strstr(s, "ERROR");
	    ptr0 = strstr(s, str);
	    if (ptr5) {
		res = GSM_POWER_DOWN;
		break;
	    } else if (ptr4) {
		res = GSM_CLOSED;
		break;
	    } else if (ptr3) {
		res = GSM_DEACT;
		break;
	    } else if (ptr2) {
		res = GSM_FAIL;
		break;
	    } else if (ptr1) {
		res = GSM_ERROR;
		break;
	    } else if (ptr0) {
		res = GSM_OK;
		break;
	    }
	}
    }

    clr_timeout();
    return res;
}


/* �������� ������ � ������ ������ ���� ��� �������� */
bool gsm_wait_for_ip_addr(char *str, int time_ms)
{
    char *ptr0, *ptr1, *ptr2, *ptr3, *ptr4, *ptr5;
    char *s = rx_struct.buf;	/* ��������� �� ����� */
    int res = GSM_ERROR;
    int n;


    set_timeout(time_ms);
    while (!is_timeout()) {

	n = gsm_get_str(s, 40);

	/* ���������� ��� � ������  �� ������ ������� */
	if (n) {
	    /*  ����� ��������� ��� ��������� ������ */
	    printf("%s ", s);
	    ptr5 = strstr(s, "0.0.0.0");
	    ptr4 = strstr(s, "CLOSED");
	    ptr3 = strstr(s, "DEACT");
	    ptr2 = strstr(s, "FAIL");
	    ptr1 = strstr(s, "ERROR");
	    ptr0 = parse_ip_addr(s);
	    if (ptr5) {
		res = GSM_NO_ADDR;
	    } else if (ptr4) {
		res = GSM_CLOSED;
		break;
	    } else if (ptr3) {
		res = GSM_DEACT;
		break;
	    } else if (ptr2) {
		res = GSM_FAIL;
		break;
	    } else if (ptr1) {
		res = GSM_FAIL;
		break;
	    } else if (ptr0) {
		strcpy(str, ptr0);
		res = GSM_OK;
		break;
	    }
	}
    }
    clr_timeout();
    return res;
}

/* �������� ������ � ������ */
bool gsm_wait_for_gsm_coord(gps_data_t * data, int time_ms)
{
    char *ptr0, *ptr1, *ptr2, *ptr3, *ptr4;
    char *s = rx_struct.buf;	/* ��������� �� ����� */
    int res = GSM_ERROR;
    int n;

    set_timeout(time_ms);
    while (!is_timeout()) {

	n = gsm_get_str(s, 40);

	/* ���������� ��� � ������  �� ������ ������� */
	if (n) {
	    /*  ����� ��������� ��� ��������� ������ */
	    printf("%s ", s);
	    ptr4 = strstr(s, "CLOSED");
	    ptr3 = strstr(s, "DEACT");
	    ptr2 = strstr(s, "FAIL");
	    ptr1 = strstr(s, "ERROR");
	    ptr0 = strstr(s, "OK");
	    if (ptr4) {
		res = GSM_CLOSED;
		break;
	    } else if (ptr3) {
		res = GSM_DEACT;
		break;
	    } else if (ptr2) {
		res = GSM_FAIL;
		break;
	    } else if (ptr1) {
		res = GSM_FAIL;
		break;
	    } else if (ptr0) {
		parse_loc_string(s, data);
		res = GSM_OK;
		break;
	    }
	}
    }
    clr_timeout();
    return res;
}


/**
 * ������������ �������� ��������� - ������ �������� 1� ��� ��������� ������ 
 */
void gsm_sim900_on_off(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* ������ ������� 4.2 V  */
    GPIO_InitStructure.GPIO_Pin = GSMON;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PWRKEY;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* �������� ������� �� VBAT */
    GPIO_SetBits(GPIOB, GSMON);
    vTaskDelay(750);

    /* ������ ������� -\_/- �� PWRKEY - �� ����� ����� ��� _/-\_  */
    led_on(LED2);
    GPIO_ResetBits(GPIOB, PWRKEY);
    vTaskDelay(50);
    GPIO_SetBits(GPIOB, PWRKEY);
    vTaskDelay(500);
    GPIO_ResetBits(GPIOB, PWRKEY);
    led_off(LED2);
}


/**
 * ������� ������ - �������� ������ � ������ � ���������� �������
 */
int gsm_send_str(char *str)
{
    PRINTF("\nSend :%s", str);
    return gsm_send_buf((u8 *) str, strlen(str));
}

/* �������� ������ ��-�����������  - ����������� � ��������� ����� ������ */
int gsm_send_buf(u8 * buf, int len)
{
    int i = 0;
    u8 byte;

    /* ���� ������� �� ��������� */
    while (i < len) {
	byte = buf[i++];
	xQueueSend(tx_queue, &byte, portMAX_DELAY);
    }
    /* �������� ���������� */
    USART_ITConfig(GSM_USART, USART_IT_TXE, ENABLE);
    return i;
}


/**
 * �������� ������ ������ � ��������
 */
void gsm_flush(buf_type_t t)
{
    if (t == GSM_BUF_IN) {
	xQueueReset(rx_queue);
    } else {
	xQueueReset(tx_queue);
    }
    memset(&rx_struct, 0, sizeof(rx_struct));
}


/*******************************************************************************
 *      �������, ����������� ��������� ��
 ******************************************************************************/

/**
 * ��������� ������ SIM900
 */
static int OnGsmPowerOnState(void)
{
    int res;
    res = sim900_power_on();
    return res;
}

/**
 * ������������� GSM ����� ���������
 */
static int OnGsmInitState(void)
{
    int res;
    res = sim900_init();
    return res;
}

/**
 * ����������� � ����
 */
static int OnGsmNetOnState(void)
{
    int res;
    res = sim900_gsm_on();
    return res;
}

/**
* ���� ���������� � �������� IP �����
 */
static int OnGsmConnectState(void)
{
    int res;
    gps_data_t data;

    do {
	/* ����������� � GSM ����� */
	res = sim900_wait_connect();
	if (res != GSM_OK) {
	    break;
	}
	/* ������� �����  */
	vTaskDelay(TASK_DELAY_TIME);

	/* ����������� - ������� IP ����� */
	sim900_get_ip_addr();

	/* ��������� �������� ��������� */
	sim900_get_coord(&data);

	/* ����������� � ��������  */
	res = sim900_tcp_connect();
	if (res != GSM_OK) {
	    break;
	}

    } while (0);
    return res;
}

/**
 * ������� ������. ������� �������� �� ����. ����� � ��������� ������
 */
static int OnGsmSendDataState(void)
{
    int res = 0;
    
    
  /* ���� �������� �������� ������ */
  if (xSemaphoreTake(gsm_tx_sem, 1)) {
    if(gsm_info_msg.info_len <= GSM_TX_STRING_SIZE && gsm_info_msg.info_len != 0) {
      printf("Send data %d bytes:\r\n", gsm_info_msg.info_len);
      
      print_data_hex(gsm_info_msg.info_buf, gsm_info_msg.info_len);
      res = sim900_send_data(gsm_info_msg.info_buf, gsm_info_msg.info_len);    
      vTaskDelay(250);
    }
  }
      return res;
}

/**
  * @brief  This function handles USART3 global interrupt request.
  * @param  None
  * @retval None
  */
void UART4_IRQHandler(void)
{
    u8 byte;
    volatile u32 status;
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    status = GSM_USART->SR;

    /* Read one byte from the receive data register */
    if (status & USART_FLAG_RXNE) {
	byte = USART_ReceiveData(GSM_USART);

	/* �������� ���� � ������� */
	xQueueSendFromISR(rx_queue, &byte, &xHigherPriorityTaskWoken);

	/* �� ������ �������� ������ - �������� ������ */
	if (byte == '\n' && xSemaphoreGiveFromISR(gsm_rx_sem, &xHigherPriorityTaskWoken)) {
	    if (xHigherPriorityTaskWoken) {
		taskYIELD();
	    }
	}
    }

    /* Write one byte to the transmit data register */
    if (status & USART_FLAG_TXE) {

	/* ����� ���� �� ������� */
	if (xQueueReceiveFromISR(tx_queue, &byte, &xHigherPriorityTaskWoken) == pdPASS) {
	    USART_SendData(GSM_USART, byte);
	} else {
	    USART_ITConfig(GSM_USART, USART_IT_TXE, DISABLE);
	}
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

