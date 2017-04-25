/* Standard includes. */
#include <stdio.h>
#include "main.h"
#include "board.h"
#include "lsm303.h"
#include "log.h"
#include "rtc.h"


/* Task priorities. */
#define MAIN_TEST_PRIORITY				(tskIDLE_PRIORITY + 1)

/* Baud rate used by the comtest tasks. */
#define USART_TASK_BAUD_RATE		(19200)
#define GSM_TASK_BAUD_RATE		(115200)
#define GPS_TASK_BAUD_RATE		(115200)

/* The LED used by the comtest tasks. See the comtest.c file for more information. */
#define mainCOM_TEST_LED			(3)

/**
 * Задача - просто моргание светодиодом
 */
static void task3(void *p)
{
  gps_data_t data;
  char str[32];
  int i = 0;

    
    while (1) {
        gps_get_data(&data);
        sec_to_str(data.time, str);
        PRINTF("Time now: %s\n", str);
	led_toggle(LED3);
	vTaskDelay(1000);        
    }
}


int main(void)
{
    int res, sec = 1452485; 

    board_init();    /* Инициализация PLL и периферии */
    led_init();
 //   rtc_init();
    
//    board_set_sec(rtc_get_sec());

    
    /*
    usart_task_create(USART_TASK_BAUD_RATE, configMINIMAL_STACK_SIZE, MAIN_TEST_PRIORITY);
    
    res = log_mount_fs();   
    PRINTF("mount res = %d\n", res);
    res = log_create_log_file();
    PRINTF("create file res = %d\n", res);    
    log_write_log_file("test string\n");
    log_close_log_file();
    */
    
 
    /* Создадим задачу для разбора данных */    
    if (usart_task_create(USART_TASK_BAUD_RATE, configMINIMAL_STACK_SIZE, MAIN_TEST_PRIORITY) != 0) {
	PRINTF("ERROR: GPS Task create\n");
	return -1;
    }

    /* Инициализация GMS модуля - вывод принятого и переданого на экран  */
    if(gsm_task_create(GSM_TASK_BAUD_RATE, configMINIMAL_STACK_SIZE, MAIN_TEST_PRIORITY) != 0) {
	PRINTF("ERROR: GSM Task create\n");
	return -1;
    }

#if 10
    if (gps_task_create(GPS_TASK_BAUD_RATE, configMINIMAL_STACK_SIZE, MAIN_TEST_PRIORITY + 1) != 0) {
	PRINTF("ERROR: GPS Task create\n");
	return -1;
    }
#endif

    xTaskCreate(task3, "task3", configMINIMAL_STACK_SIZE, NULL, MAIN_TEST_PRIORITY + 2, NULL);
    
    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the  idle task. */
    return 0;
}

