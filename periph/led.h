#ifndef _LED_H
#define _LED_H

#include "main.h"

/** @defgroup STM32_EVAL_Exported_Types
  * @{
  */
typedef enum 
{
  LED1,
  LED2,
  LED3,
  LED4
} led_type_t;


void led_init(void);
void led_on(led_type_t led);
void led_off(led_type_t led);
void led_toggle(led_type_t led);

#endif /* led.h */
