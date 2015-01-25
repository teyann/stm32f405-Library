#ifndef _PWM_H_
#define _PWM_H_

#include <drv_exti.h>

#define RC_MIN 1020
#define RC_MAX 1980

/*
 * @brief pwm 장치 열거체
 */
typedef enum {
    PWM_1 = 0,
    PWM_2,
    MAX_PWM,
} pwmDevice_t;

/*
 * @brief pwm 하드웨어 맵핑타입 구조체
 */
typedef struct {
	extiDevice_t a[6];
} pwmExtiMap_t;

/*
 * @brief pwm 하드웨어 맵핑
 */
static const pwmExtiMap_t pwmExtiMap[] = {
	{ .a = {EXTI_DEVICE_2, EXTI_DEVICE_3, EXTI_DEVICE_4, EXTI_DEVICE_5, EXTI_DEVICE_15, 0xff} },
	{ .a = {EXTI_DEVICE_12, EXTI_DEVICE_3, EXTI_DEVICE_4, EXTI_DEVICE_5, EXTI_DEVICE_15, 0xff} },
};

/*
 * @brief pwm 타입 열거체
 */
typedef enum {
	PWM_FILTER_DISABLE = 0,
	PWM_FILTER_ENABLE,
} pwmType_t;

void pwmInit(const extiDevice_t* extiDevicePtr, pwmType_t type);
void pwmRead(uint16_t *data);

#endif
