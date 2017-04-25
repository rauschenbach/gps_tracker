#include "lsm303dlhc_drv.h"
#include "lsm303.h"
#include "main.h"



#define ACC_SENSITIVITY_2G     (1.0)	/*!< accelerometer sensitivity with 2 g full scale [LSB/mg] */
#define ACC_SENSITIVITY_4G     (2.0)	/*!< accelerometer sensitivity with 4 g full scale [LSB/mg] */
#define ACC_SENSITIVITY_8G     (4.0)	/*!< accelerometer sensitivity with 8 g full scale [LSB/mg] */
#define ACC_SENSITIVITY_16G    (12.0)	/*!< accelerometer sensitivity with 12 g full scale [LSB/mg] */


/* Инициализация акселерометра - значения в G*/
void lsm303_acc_init(void)
{
    LSM303DLHCAcc_InitTypeDef LSM303DLHCAcc_InitStructure;
    LSM303DLHCAcc_FilterConfigTypeDef LSM303DLHCFilter_InitStructure;


    /* Fill the accelerometer structure */
    LSM303DLHCAcc_InitStructure.Power_Mode = LSM303DLHC_NORMAL_MODE;
//      LSM303DLHCAcc_InitStructure.AccOutput_DataRate = LSM303DLHC_ODR_100_HZ;
    LSM303DLHCAcc_InitStructure.AccOutput_DataRate = LSM303DLHC_ODR_10_HZ;
    LSM303DLHCAcc_InitStructure.Axes_Enable = LSM303DLHC_AXES_ENABLE;
    LSM303DLHCAcc_InitStructure.AccFull_Scale = LSM303DLHC_FULLSCALE_2G;
    LSM303DLHCAcc_InitStructure.BlockData_Update = LSM303DLHC_BlockUpdate_Continous;
    LSM303DLHCAcc_InitStructure.Endianness = LSM303DLHC_BLE_LSB;
    LSM303DLHCAcc_InitStructure.High_Resolution = LSM303DLHC_HR_ENABLE;
    LSM303DLHC_AccInit(&LSM303DLHCAcc_InitStructure);	/* Configure the accelerometer main parameters */


    /* Fill the accelerometer Low Pass Filter structure */
#if 1
    LSM303DLHCFilter_InitStructure.HighPassFilter_Mode_Selection = LSM303DLHC_HPM_NORMAL_MODE;
    LSM303DLHCFilter_InitStructure.HighPassFilter_CutOff_Frequency = LSM303DLHC_HPFCF_16;
    LSM303DLHCFilter_InitStructure.HighPassFilter_AOI1 = LSM303DLHC_HPF_AOI1_DISABLE;
    LSM303DLHCFilter_InitStructure.HighPassFilter_AOI2 = LSM303DLHC_HPF_AOI2_DISABLE;

    /* Configure the accelerometer LPF main parameters */
    LSM303DLHC_AccFilterConfig(&LSM303DLHCFilter_InitStructure);
#endif
    /* Required delay for the MEMS Accelerometre: Turn-on time = 3 / Output data Rate = 3/100 = 30ms */
    delay_ms(50);
}

/**
 * Хотя значения 16 битные - компилятор не может писать в s16
 * упакованные в структуры!!!
 * Read LSM303DLHC output register, and calculate the acceleration
 * ACC=(1 / SENSITIVITY) * (out_h*256+out_l)/16 (12 bit rappresentation)
 */
void lsm303_acc_get_data(acc_data * acc_val)
{
    u8 ctrlx[2];
    u8 lo, hi;
    s16 x, y, z;
    f32 sens = ACC_SENSITIVITY_2G;

    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_L_A, 1, &lo);
    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_X_H_A, 1, &hi);
    x = ((s16) ((u16) hi << 8) + lo) >> 4;

    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_A, 1, &lo);
    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_A, 1, &hi);
    y = ((s16) ((u16) hi << 8) + lo) >> 4;

    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_A, 1, &lo);
    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_A, 1, &hi);
    z = ((s16) ((u16) hi << 8) + lo) >> 4;

    /* Read the register content */
    LSM303DLHC_Read(ACC_I2C_ADDRESS, LSM303DLHC_CTRL_REG4_A, 2, ctrlx);

    /* Little Endian Mode or FIFO mode */
    if (ctrlx[1] & 0x40) {
	sens = 4.0;
    } else {
	/* normal mode */
	/* switch the sensitivity value set in the CRTL4 */
	switch (ctrlx[0] & 0x30) {
	case LSM303DLHC_FULLSCALE_2G:
	    sens = ACC_SENSITIVITY_2G;
	    break;
	case LSM303DLHC_FULLSCALE_4G:
	    sens = ACC_SENSITIVITY_4G;
	    break;
	case LSM303DLHC_FULLSCALE_8G:
	    sens = ACC_SENSITIVITY_8G;
	    break;
	case LSM303DLHC_FULLSCALE_16G:
	    sens = ACC_SENSITIVITY_16G;
	    break;
	}
    }

    /* Получить значения mG по осям */
    acc_val->x = x * sens;
    acc_val->y = y * sens;
    acc_val->z = z * sens;
}


/* Enable and set EXTI Line2 Interrupt to the lowest priority */
void lsm303_acc_irq_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
#if 0
    /* Enable DRDY clock */
    RCC_APB2PeriphClockCmd(LSM303DLHC_DRDY_GPIO_CLK, ENABLE);

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);


    /* Mems DRDY pin configuration */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = LSM303DLHC_DRDY_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LSM303DLHC_DRDY_GPIO_PORT, &GPIO_InitStructure);

    /* Connect EXTI Line to Mems DRDY Pin */
    SYSCFG_EXTILineConfig(LSM303DLHC_DRDY_EXTI_PORT_SOURCE, LSM303DLHC_DRDY_EXTI_PIN_SOURCE);
    EXTI_InitStructure.EXTI_Line = LSM303DLHC_DRDY_EXTI_LINE;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}



/* Инициализация компаса */
void lsm303_comp_init(void)
{
    LSM303DLHCMag_InitTypeDef LSM303DLHC_InitStruct;

    /* Заполним структуру */
    LSM303DLHC_InitStruct.Temperature_Sensor = LSM303DLHC_TEMPSENSOR_ENABLE;	/* Temperature sensor enable/disable */
    LSM303DLHC_InitStruct.MagOutput_DataRate = LSM303DLHC_ODR_30_HZ;	/* OUT data rate */
    LSM303DLHC_InitStruct.Working_Mode = LSM303DLHC_CONTINUOS_CONVERSION;	/* operating mode */
    LSM303DLHC_InitStruct.MagFull_Scale = LSM303DLHC_FS_1_3_GA;	/* Full Scale selection */
    LSM303DLHC_MagInit(&LSM303DLHC_InitStruct);
}

void lsm303_comp_get_data(acc_data * data)
{
    u8 lo, hi, gain;
    s16 x, y, z;
    int t;
    f32 sens_xy = 1100.0, sens_z = 980.0;

    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_H_M, 1, &hi);
    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_X_L_M, 1, &lo);
    x = ((s16) ((u16) hi << 8) + lo);

    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_M, 1, &hi);
    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_M, 1, &lo);
    z = ((s16) ((u16) hi << 8) + lo);

    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_H_M, 1, &hi);
    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Y_L_M, 1, &lo);
    y = ((s16) ((u16) hi << 8) + lo);


    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_H_M, 1, &hi);
    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_OUT_Z_L_M, 1, &lo);
    t = ((s16) ((u16) hi << 8) + lo);

    /* Прочитаем регистр усиления для магнитного поля */
    LSM303DLHC_Read(MAG_I2C_ADDRESS, LSM303DLHC_CRB_REG_M, 1, &gain);
    switch (gain) {

	/*!< Full scale = ±1.9 Gauss */
    case LSM303DLHC_FS_1_9_GA:
	sens_xy = 855.0;
	sens_z = 760.0;
	break;

	/*!< Full scale = ±2.5 Gauss */
    case LSM303DLHC_FS_2_5_GA:
	sens_xy = 670.0;
	sens_z = 600.0;
	break;

	/*!< Full scale = ±4.0 Gauss */
    case LSM303DLHC_FS_4_0_GA:
	sens_xy = 450.0;
	sens_z = 400.0;
	break;

	/*!< Full scale = ±4.7 Gauss */
    case LSM303DLHC_FS_4_7_GA:
	sens_xy = 400.0;
	sens_z = 355.0;
	break;

	/*!< Full scale = ±5.6 Gauss */
    case LSM303DLHC_FS_5_6_GA:
	sens_xy = 330.0;
	sens_z = 295.0;
	break;

	/*!< Full scale = ±8.1 Gauss */
    case LSM303DLHC_FS_8_1_GA:
	sens_xy = 230.0;
	sens_z = 205.0;
	break;

	/*!< Full scale = ±1.3 Gauss */
    case LSM303DLHC_FS_1_3_GA:
    default:
	break;
    }

    data->x = (int) x *1000 / sens_xy;
    data->y = (int) y *1000 / sens_xy;
    data->z = (int) z *1000 / sens_z;

    t *= 10;
    t >>= 7;
    t += 220;
    data->t = t;
}
