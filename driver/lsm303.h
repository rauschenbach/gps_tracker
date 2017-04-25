#ifndef _LSM303_H
#define _LSM303_H

#include "main.h"

void lsm303_acc_init(void);
void lsm303_acc_get_data(acc_data*);
void lsm303_acc_irq_init(void);
void lsm303_comp_init(void);
void lsm303_comp_get_data(acc_data*);

#endif				/* acc.h */
