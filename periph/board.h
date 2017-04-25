#ifndef _BOARD_H
#define _BOARD_H

#include "main.h"


void board_gpio_init(void);
void board_init(void);
void board_power_on(void);
void vApplicationTickHook(void);
void set_timeout(int);
bool is_timeout(void);
void clr_timeout(void);

int  board_get_sec(void);
void board_set_sec(int);

void vSmartDelay(u32);

#define delay_ms(x)	vSmartDelay(x * 1000)
#define delay_sec(x)	delay_ms(x * 1000)


#endif /* board.h */
