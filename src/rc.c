#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_uart.h>
#include <pwm.h>
#include <rc.h>

#ifndef bool
typedef uint8_t bool;
#define false (bool) 0
#define true (bool) 1
#define NULL ((void *)0)
#endif

/*
 * @brief 사용할 외부인터럽트 장치를 저장할 포인터
 */
static const extiDevice_t* rcExtiDevicePtr = NULL; // NULL

/*
 * @brief 사용할 수신기의 타입을 저장할 변수
 */
static rcType_t rcType = 0;

/*
 * @brief 수신기 초기화
 * @param rcDevice: 수신기 장치 열거체
 * @param type: 수신기 초기화를 위한 열거체
 * @retval 없음
 */
void rcInit(rcDevice_t rcDevice, rcType_t type) { // rcDevice
	rcType = type; // PWM, PPM
	rcExtiDevicePtr = pwmExtiMap[rcDevice].a; // a[] = { EXTI1, TIM2, TIM3, TIM4 }, pointer value
	switch(rcType) {
	case PWM:
		pwmInit(rcExtiDevicePtr, PWM_FILTER_DISABLE);
		break;
	case PPM:
		break;
	case S_BUS:
		break;
	}
}

/*
 * @brief 수신기 읽기
 * @param data: 5개의 데이터 반환을 위한 포인터
 * @retval 없음
 */
void rcRead(uint16_t *data) {
	switch(rcType) {
	case PWM:
		pwmRead(data);
		break;
	case PPM:
		break;
	case S_BUS:
		break;
	}
}
