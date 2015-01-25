#ifndef _EXTI_H_
#define _EXTI_H_

#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>

/*
 * @brief 외부인터럽트 장치 열거체
 */
typedef enum {
	EXTI_DEVICE_0 = 0,
	EXTI_DEVICE_1,
	EXTI_DEVICE_2,
	EXTI_DEVICE_3,
	EXTI_DEVICE_4,
	EXTI_DEVICE_5,
	EXTI_DEVICE_6,
	EXTI_DEVICE_7,
	EXTI_DEVICE_8,
	EXTI_DEVICE_9,
    EXTI_DEVICE_10,
    EXTI_DEVICE_11,
    EXTI_DEVICE_12,
    EXTI_DEVICE_13,
    EXTI_DEVICE_14,
    EXTI_DEVICE_15,
    MAX_EXTI_DEVICE,
} extiDevice_t;

/*
 * @brief 외부인터럽트 하드웨어 맵핑을 위한 구조체
 */
typedef struct {
	GPIO_TypeDef *gpio;
	uint16_t pin;
	uint8_t portSource;
	uint8_t pinSource;
	uint32_t exti;
	uint8_t irq;
	uint32_t periph;
} extiHardwareMap_t;

/*
 * @brief 외부인터럽트 하드웨어 맵핑
 */
static const extiHardwareMap_t extiHardwareMap[] = {
	{ 0, GPIO_Pin_0, 0xff, EXTI_PinSource0, EXTI_Line0, EXTI0_IRQn, 0xff },
	{ 0, GPIO_Pin_1, 0xff, EXTI_PinSource1, EXTI_Line1, EXTI1_IRQn, 0xff },
	{ GPIOD, GPIO_Pin_2, EXTI_PortSourceGPIOD, EXTI_PinSource2, EXTI_Line2, EXTI2_IRQn, RCC_AHB1Periph_GPIOD }, // EXTI_2
	{ GPIOC, GPIO_Pin_3, EXTI_PortSourceGPIOC, EXTI_PinSource3, EXTI_Line3, EXTI3_IRQn, RCC_AHB1Periph_GPIOC }, // EXTI_3
	{ GPIOA, GPIO_Pin_4, EXTI_PortSourceGPIOA, EXTI_PinSource4, EXTI_Line4, EXTI4_IRQn, RCC_AHB1Periph_GPIOA }, // EXTI_4
	{ GPIOA, GPIO_Pin_5, EXTI_PortSourceGPIOA, EXTI_PinSource5, EXTI_Line5, EXTI9_5_IRQn, RCC_AHB1Periph_GPIOA }, // EXTI_5
	{ 0, GPIO_Pin_6, 0xff, EXTI_PinSource6, EXTI_Line6, EXTI9_5_IRQn, 0xff },
	{ 0, GPIO_Pin_7, 0xff, EXTI_PinSource7, EXTI_Line7, EXTI9_5_IRQn, 0xff },
	{ 0, GPIO_Pin_8, 0xff, EXTI_PinSource8, EXTI_Line8, EXTI9_5_IRQn, 0xff },
	{ 0, GPIO_Pin_9, 0xff, EXTI_PinSource9, EXTI_Line9, EXTI9_5_IRQn, 0xff },
	{ 0, GPIO_Pin_10, 0xff, EXTI_PinSource10, EXTI_Line10, EXTI15_10_IRQn, 0xff },
	{ 0, GPIO_Pin_11, 0xff, EXTI_PinSource11, EXTI_Line11, EXTI15_10_IRQn, 0xff },
	{ GPIOC, GPIO_Pin_12, EXTI_PortSourceGPIOC, EXTI_PinSource12, EXTI_Line12, EXTI15_10_IRQn, RCC_AHB1Periph_GPIOC }, // EXTI_12
	{ 0, GPIO_Pin_13, 0xff, EXTI_PinSource13, EXTI_Line13, EXTI15_10_IRQn, 0xff },
	{ 0, GPIO_Pin_14, 0xff, EXTI_PinSource14, EXTI_Line14, EXTI15_10_IRQn, 0xff },
	{ GPIOB, GPIO_Pin_15, EXTI_PortSourceGPIOB, EXTI_PinSource15, EXTI_Line15, EXTI15_10_IRQn, RCC_AHB1Periph_GPIOB }, // EXTI_15
};

/*
 * @brief 외부인터럽트 초기화 타입 구조체
 */
typedef struct
{
	EXTITrigger_TypeDef Trigger;
	uint8_t PreemptionPriority;
	uint8_t SubPriority;
} extiInitTypeDef_t;

/*
 * @brief 외부인터럽트  콜백 함수
 */
typedef void (*extiFuncPtr_t) (extiDevice_t);

void extiInit(extiDevice_t extiDevice, extiInitTypeDef_t* extiInitStruct, extiFuncPtr_t extiFunc_);
void extiChannelMapping(extiDevice_t extiDevice, uint8_t channel);

#endif
