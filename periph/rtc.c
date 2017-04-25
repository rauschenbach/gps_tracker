#include <string.h>
#include <stdio.h>
#include <time.h>
#include "main.h"
#include "rtc.h"

#define RTC_CLOCK_SOURCE_LSI	/* LSE used as RTC source clock */
#define REG_DATA        0xA55A        


/* инициализация RTC */
void rtc_init(void)
{
    if (BKP_ReadBackupRegister(BKP_DR1) != REG_DATA) {
	int presc;

	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Reset Backup Domain */
	BKP_DeInit();

#if defined RTC_CLOCK_SOURCE_LSE
	presc = 32767;

	/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);

	/* Wait till LSE is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {}

	/* Select LSE as RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#else
	presc = 37000;
	/* Enable the LSI OSC - The RTC Clock may varies due to LSI frequency dispersion. */
	RCC_LSICmd(ENABLE);

	/* Wait till LSI is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}

	/* Select the RTC Clock Source */
	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);


#endif
	/* Enable RTC Clock */
	RCC_RTCCLKCmd(ENABLE);

	/* Wait for RTC registers synchronization */
	RTC_WaitForSynchro();

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();


	/* Set RTC prescaler: set RTC period to 1sec */
	RTC_SetPrescaler(presc);	/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();

	BKP_WriteBackupRegister(BKP_DR1, REG_DATA);
    } else {
	//==================== ВАЖНОЕ ДОПОЛНЕНИЕ =======================
	//- без него не пишется в счетчики RTC (взято из RTC_Configuration)   
	/* Enable PWR and BKP clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);

	/* Wait for RTC registers synchronization  */
	RTC_WaitForSynchro();

	/* Enable the RTC Second */
	RTC_ITConfig(RTC_IT_SEC, ENABLE);
	// Wait until last write operation on RTC registers has finished 
	RTC_WaitForLastTask();
    }
}


/* Установить время и дату */
void rtc_set_time(void *p)
{
    struct tm *t0;
    long sec;

    if (p != NULL) {
	t0 = (struct tm *) p;

	sec = mktime(t0);

	/* Записать секунды */
	RTC_SetCounter(sec);

	/* Wait until last write operation on RTC registers has finished */
	RTC_WaitForLastTask();	//!!!??? (виснет)
    }
}

/* Получить время */
void rtc_get_time(void *p)
{
    long sec;
    struct tm *t0;

    sec = RTC_GetCounter();
    t0 = gmtime((time_t *) & sec);	/* Переведем время в tm_time */

    if (p != NULL) {
	memcpy(p, t0, sizeof(struct tm));
    }
}

void rtc_set_sec(int s)
{
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Change the current time */
    RTC_SetCounter(s);

    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();	//!!!??? (виснет)
}

int rtc_get_sec(void)
{
    return RTC_GetCounter();
}
