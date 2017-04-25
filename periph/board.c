#include "main.h"
#include "board.h"
#include "gsm.h"

static struct time_struct {
    __IO uint32_t TimingDelay;
    __IO uint32_t sec;
    __IO uint16_t ms;
} board_time;


/*
 * Configure the clocks, GPIO and other peripherals as required by the demo.
 */
void board_init(void)
{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

    board_time.TimingDelay = 0;

    // ===== АКТИВИЗАЦИЯ НЕОБХОДИМОЙ ПЕРИФЕРИИ =====
    // Enable GPIOA, GPIOB, GPIOC 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

/*	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIO_SetBits(GPIOB, GPIO_Pin_5); // | GPIO_Pin_5  -DRDY от акселерометра

*/
    // для использования выводов JTAG !!!!! 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* Set the Vector Table base address at 0x08000000 */
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00);

    /* Configure the Priority Group to 2 bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);


    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA;
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA;
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA;
}

/* Включение GSM и GPS */
void board_power_on(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Подача питания 4.2 V  */
    GPIO_InitStructure.GPIO_Pin = GSMON;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = PWRKEY;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Включаем питание на VBAT */
    GPIO_SetBits(GPIOB, GSMON);
}


/* Decrement the TimingDelay variable */
void vApplicationTickHook(void)
{
    if (board_time.ms++ % 1000 == 0) {
/*
	gps_data_t gps;
	gps_get_data(&gps);

	// Если точных данных нет - ставим свой таймер
	if (gps.status < FIX_2D) {
	    gps.time = board_time.sec;
	    gps_set_data(&gps);
	}
*/
	board_time.sec++;
    }

    if (board_time.TimingDelay != 0x00) {
	board_time.TimingDelay--;
    }
}


/* Установить таймаут в мс */
void set_timeout(int ms)
{
    board_time.TimingDelay = ms;
//    SysTick->VAL = (uint32_t) 0x0;    //???
}

/* Проверить, прошел таймаут или не */
bool is_timeout(void)
{
    volatile bool res = false;

    if (board_time.TimingDelay == 0) {
	res = true;
    }
    return res;
}

void clr_timeout(void)
{
    board_time.TimingDelay = 0;
}

int board_get_sec(void)
{
    return board_time.sec;
}

void board_set_sec(int sec)
{
    board_time.ms = 10;
    board_time.sec = sec;
}


/** 
 * Умная задержка - определяет в каом контексте выполняется
 */
/*
void vSmartDelay(u32 ms)
{
	set_timeout(ms);
	while(!is_timeout());
        clr_timeout();
}
*/
void vSmartDelay(u32 ulCount)
{
    __asm("    subs    r0, #1\n" "    bne.n   vSmartDelay\n");
}
