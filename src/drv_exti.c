#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_exti.h>

/*
 * @brief 외부인터럽트  콜백 함수 배열
 */
static extiFuncPtr_t extiFuncPtr[16];

/*
 * @brief 외부인터럽트에서 호출할 데이터 맵핑
 */
static uint8_t extiChannelMap[16] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*
 * @brief 외부인터럽트 채널 맵핑
 * @param extiDevice: 외부인터럽트 장치 열거체
 * @param channel: 외부인터럽트 초기화를 위한 구조체 포인터
 * @retval 없음
 */
void extiChannelMapping(extiDevice_t extiDevice, uint8_t channel) {
	extiChannelMap[extiDevice] = channel;
}

/*
 * @brief 외부인터럽트 초기화
 * @param extiDevice: 외부인터럽트 장치 열거체
 * @param extiInitStruct: 외부인터럽트 초기화를 위한 구조체 포인터
 * @param extiFuncPtr_: 외부인터럽트 ISR에서 호출할 함수 포인터
 * @retval 없음
 */
void extiInit(extiDevice_t extiDevice, extiInitTypeDef_t* extiInitStruct, extiFuncPtr_t extiFuncPtr_) {
	// clock 활성화
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHB1PeriphClockCmd(extiHardwareMap[extiDevice].periph, ENABLE);

	// gpio 설정
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = extiHardwareMap[extiDevice].pin;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_Init(extiHardwareMap[extiDevice].gpio, &GPIO_InitStructure);

	// exti핀 할당
	SYSCFG_EXTILineConfig(extiHardwareMap[extiDevice].portSource, extiHardwareMap[extiDevice].pinSource);

	// exti 설정
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = extiHardwareMap[extiDevice].exti;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = extiInitStruct->Trigger;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	// nvic 설정
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = extiHardwareMap[extiDevice].irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = extiInitStruct->PreemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = extiInitStruct->SubPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// 호출할 함수 설정
	extiFuncPtr[extiDevice] = extiFuncPtr_;
}

/*
 * @brief 외부인터럽트 인터럽트 핸들러
 * @note 1020~1980의 데이터만 지속적으로 받도록 설정이 되어 있으므로
 * 		  데이터의 범위가 넓은 수신기의 경우 적절하게 조절해서 사용하도록 .. 실제 데이터는 1100~1900으로 가공해서 사용예정
 * 		  따라서, 권장 사양은 1050~1950이고, 최대사양은 1020~1980, 최소사양은 1100~1900
 * @param channel: x는 1에서  5의 숫자가 가능함
 * @retval 없음
 */
static void extiHandler(extiDevice_t channel) {
	uint8_t chan = extiChannelMap[channel]; //EXTI값으로 Channel값 읽어오기, 예) exti2 -> THR
	if(chan == 0xff) {
		return;
	}
	extiFuncPtr[channel](chan);
}

void EXTI0_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line0) == SET) { //인터럽트 플래그가 set되었을경우
		EXTI_ClearITPendingBit(EXTI_Line0); //인터럽트 플래그 clear
		extiHandler(EXTI_DEVICE_0); //pulse 시간계산을 위한 함수 호출
	}
}

void EXTI1_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line1) == SET) {
		EXTI_ClearITPendingBit(EXTI_Line1);
		extiHandler(EXTI_DEVICE_1);
	}
}

void EXTI2_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line2) == SET) {
		EXTI_ClearITPendingBit(EXTI_Line2);
		extiHandler(EXTI_DEVICE_2);
	}
}

void EXTI3_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line3) == SET) {
		EXTI_ClearITPendingBit(EXTI_Line3);
		extiHandler(EXTI_DEVICE_3);
	}
}

void EXTI4_IRQHandler(void) {
	if(EXTI_GetITStatus(EXTI_Line4) == SET) {
		EXTI_ClearITPendingBit(EXTI_Line4);
		extiHandler(EXTI_DEVICE_4);
	}
}

void EXTI9_5_IRQHandler(void) {
	uint8_t i = 5;
	for(; i <= 9; i++) {//5 ~ 9
		if(EXTI_GetITStatus(1 << i) == SET) { // 1<<5 ~ 1<<9, EXTI_Line5 ~ EXTI_Line9
			EXTI_ClearITPendingBit(1 << i);
			extiHandler(i); //5 ~ 9, EXTI5 ~ EXTI9
		}
	}
}

void EXTI15_10_IRQHandler(void) {
	uint8_t i = 10;
	for(; i <= 15; i++) {
		if(EXTI_GetITStatus(1 << i) == SET) {
			EXTI_ClearITPendingBit(1 << i);
			extiHandler(i);
		}
	}
}
