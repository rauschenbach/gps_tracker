#include "sim900.h"
#include "string.h"
#include "main.h"




/*
 * ��������� ������ -  �� 130 ������ �� �������� ������ ������� 
*/
int sim900_power_on(void)
{
    int res = GSM_ERROR;

    gsm_sim900_on_off();
    printf("\nWait for \"Call Ready\"...\r\n");
    res = gsm_wait_for_str("Call Ready", WAIT_TIME_10SEC);
    vTaskDelay(DELAY_CMD_MS);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */
    return res;
}

/*
 * ������������� ����� ���������
*/
int sim900_init(void)
{
    int res = GSM_OK;
    do {
	/* ���������� ��� */
	gsm_send_str("ATE0\r\n");
	res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
	vTaskDelay(DELAY_CMD_MS);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
	if (res != GSM_OK) {
	    break;
	}

	/* ������ ���� �������� ������� */
	gsm_send_str("AT+GSMBUSY=1\r\n");
	res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
	vTaskDelay(DELAY_CMD_MS);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
	if (res != GSM_OK) {
	    break;
	}

	/* ������ ������ ������� ���� */
	gsm_send_str("AT+CSQ\r\n");
	res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
	vTaskDelay(DELAY_CMD_MS);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
	if (res != GSM_OK) {
	    break;
	}

	/* � ������ ��������� ���������� */
	gsm_send_str("AT+COPS?\r\n");
	res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
	vTaskDelay(DELAY_CMD_MS);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
	if (res != GSM_OK) {
	    break;
	}

	gsm_send_str("AT+GMR\r\n");
	res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
	vTaskDelay(DELAY_CMD_MS);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    } while (0);

    return res;
}

/* �������� GPRS */
int sim900_gsm_on(void)
{
    int res;
#if 0
    gsm_send_str("AT+CIPCSGP=1,\"internet\",\"gdata\",\"gdata\"\r\n");
    res = gsm_wait_for_str("OK", WAIT_TIME_10SEC * 2);
    printf("\nWait for 2 sec\r\n");
    vTaskDelay(CONNECT_TIMEOUT);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */
#else
    gsm_send_str("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n");
    res = gsm_wait_for_str("OK", WAIT_TIME_10SEC * 2);
    printf("\nWait for 2 sec\r\n");
    vTaskDelay(CONNECT_TIMEOUT);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    gsm_send_str("AT+SAPBR=3,1,\"APN\",\"gdata\"\r\n");
    res = gsm_wait_for_str("OK", WAIT_TIME_10SEC * 2);
    printf("\nWait for 2 sec\r\n");
    vTaskDelay(CONNECT_TIMEOUT);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

#endif
    return res;
}

/* ���������� ����������� GPRS */
int sim900_wait_connect(void)
{
    int res;

    gsm_send_str("AT+SAPBR=1,1\r\n");
    res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
    printf("\nWait for 2 sec\r\n");
    vTaskDelay(CONNECT_TIMEOUT);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    if (res != GSM_OK) {
	gsm_send_str("AT+CGATT?\r\n");
	vTaskDelay(CONNECT_TIMEOUT);
	res = gsm_wait_for_str("+CGATT:", WAIT_TIME_10SEC);
	printf("\nWait for 2 sec\r\n");
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
    }

    return res;
}

/* �������� IP ����� */
int sim900_get_ip_addr(void)
{
    int res, i = 3;
    char str[128];

    do {
	gsm_send_str("AT+SAPBR=2,1\r\n");	/*  ���� ����� ��������� IP ����� */
	vTaskDelay(CONNECT_TIMEOUT);

	res = gsm_wait_for_ip_addr(str, WAIT_TIME_10SEC / 2);
	gsm_flush(GSM_BUF_IN);	/* ������ �������� */
	if (res == GSM_OK) {
	    printf("My IP Addr: %s\r\n", str);
	    break;
	}
	vTaskDelay(CONNECT_TIMEOUT);
    } while (i--);

    return res;
}


/**
 *  ��������� ���������� �� ������� �������� 
 */
int sim900_get_coord(gps_data_t * data)
{
    bool res;
    gsm_send_str("AT+CIPGSMLOC=1,1\r\n");
    vTaskDelay(CONNECT_TIMEOUT);
    res = gsm_wait_for_gsm_coord(data, WAIT_TIME_10SEC);
    printf("\nWait for 2 sec\r\n");
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */
    vTaskDelay(CONNECT_TIMEOUT);
    return res;
}


/*****************************************************************************
 * ����������� � ������� �� IP � �����
 * ������ ���������� �� ����� �������
 
 *****************************************************************************/
int sim900_tcp_connect(void)
{
    int res;

    /* �������� ������ �� ������ */
    gsm_send_str(SERVER_STRING);
    vTaskDelay(DELAY_CMD_MS * 2);

    res = gsm_wait_for_str("CONNECT OK", WAIT_TIME_10SEC);
    vTaskDelay(250);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    return res;
}

/* ������� ���������� */
int sim900_tcp_close(void)
{
    int res;

    /* close socket forcibly without ACK */
    gsm_send_str("AT+CIPCLOSE=1\r\n");
    res = gsm_wait_for_str("CLOSE", WAIT_TIME_10SEC);
    vTaskDelay(DELAY_CMD_MS);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */
    return res;
}


/* �������� ������ ������ */
int sim900_send_data(u8 * buf, u16 len)
{
    int res;
    char str[32];

    sprintf(str, "AT+CIPSEND=%d\r\n", len);
    gsm_send_str(str);
    vTaskDelay(DELAY_CMD_MS);	/* ���� ������� � ������������ */
    res = gsm_wait_for_str(">", WAIT_TIME_10SEC);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    if (res == GSM_OK) {
	gsm_send_buf((u8 *) buf, len);
	vTaskDelay(DELAY_CMD_MS);
	res = gsm_wait_for_str("SEND OK", WAIT_TIME_10SEC * 2);
    } else if (res == GSM_DEACT) {
	gsm_send_str("AT+CGATT?\r\n");
	vTaskDelay(CONNECT_TIMEOUT);
	gsm_wait_for_str("+CGATT:", WAIT_TIME_10SEC);
	printf("\nWait for 2 sec\r\n");
	gsm_flush(GSM_BUF_IN);
    }


    gsm_flush(GSM_BUF_IN);	/* ������ �������� */
    return res;
}


/* ��������� GPRS */
int sim900_gsm_off(void)
{
    int res;

    gsm_send_str("AT+SAPBR=0,1\r\n");
    res = gsm_wait_for_str("OK", WAIT_TIME_10SEC);
    if (res == GSM_OK) {
	printf("\r\nDisconnect OK\r\n");
    }

    vTaskDelay(DELAY_CMD_MS);
    gsm_flush(GSM_BUF_IN);	/* ������ �������� */

    return res;
}
