#include "led.h"

/* LED */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_StructInit(&GPIO_InitStructure);
   
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* led 3 è led4 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);   

    GPIO_SetBits(GPIOC, GPIO_Pin_4);
    GPIO_SetBits(GPIOC, GPIO_Pin_5);
    GPIO_SetBits(GPIOB, GPIO_Pin_0);
    GPIO_SetBits(GPIOB, GPIO_Pin_1);   
}


void led_on(led_type_t led)
{
    switch (led) {
    case LED1:
      	GPIO_ResetBits(GPIOC, GPIO_Pin_4);
	break;

    case LED2:
	GPIO_ResetBits(GPIOC, GPIO_Pin_5);
	break;

    case LED3:
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
	break;

    case LED4:
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	break;
        
    default:
	break;
    }
}

void led_off(led_type_t led)
{
    switch (led) {
    case LED1:
      	GPIO_SetBits(GPIOC, GPIO_Pin_4);
	break;

    case LED2:
	GPIO_SetBits(GPIOC, GPIO_Pin_5);
	break;

    case LED3:
	GPIO_SetBits(GPIOB, GPIO_Pin_0);
	break;

    case LED4:
	GPIO_SetBits(GPIOB, GPIO_Pin_1);
	break;

    default:
	break;
    }
}

void led_toggle(led_type_t led)
{
    switch (led) {
    case LED1:
      	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) ? GPIO_ResetBits(GPIOC, GPIO_Pin_4) : GPIO_SetBits(GPIOC, GPIO_Pin_4);
	break;

    case LED2:
      	GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) ? GPIO_ResetBits(GPIOC, GPIO_Pin_5) : GPIO_SetBits(GPIOC, GPIO_Pin_5);
	break;

    case LED3:
	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) ? GPIO_ResetBits(GPIOB, GPIO_Pin_0) : GPIO_SetBits(GPIOB, GPIO_Pin_0);
	break;

    case LED4:
	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) ? GPIO_ResetBits(GPIOB, GPIO_Pin_1) : GPIO_SetBits(GPIOB, GPIO_Pin_1);
	break;      

    default:
	break;
    }
}
