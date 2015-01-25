#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_exti.h>
#include <pwm.h>
#include <rc.h>
#include <system.h>

#ifndef bool
typedef uint8_t bool;
#define false (bool) 0
#define true (bool) 1
#endif

static volatile uint32_t rcRising[RC_CHANNEL_MAX] = {0, };
static volatile uint32_t rcFalling[RC_CHANNEL_MAX] = {0, };
static volatile uint16_t rcRawData[RC_CHANNEL_MAX] = {1000, 1000, 1000, 1000, 1000};
static volatile bool rcDataVaild[RC_CHANNEL_MAX] = {true, true, true, true, true};

static const extiDevice_t* extiChannel = 0; //NULL

void pwmHandler(extiDevice_t extiDevice);

/*
 * @brief pwm 초기화
 * @note 필터 설정 부분 아직 구현안함, 플래그를 통해 구현하도록..
 * @param extiDevicePtr: 외부인터럽트 장치의 포인터
 * @param type: 수신기 초기화를 위한 열거체
 * @retval 없음
 */
void pwmInit(const extiDevice_t* extiDevicePtr, pwmType_t type) {
	extiChannel = extiDevicePtr; //사용할 수신기의 채널을 저장
	uint8_t i = 0;
	for(i = 0; i < MAX_EXTI_DEVICE; i++) {
		if(extiDevicePtr[i] == 0xff) {
			break;
		}

		extiDevice_t extiDevice = extiDevicePtr[i];
		extiChannelMapping(extiDevice, i);

		extiInitTypeDef_t extiInitStructure;
		extiInitStructure.Trigger = EXTI_Trigger_Rising_Falling;
		extiInitStructure.PreemptionPriority = 1; //기본적으로 1, 1로 설정하고 나중에 인터럽트가 많아지면 상황에 맞춰 조절
		extiInitStructure.SubPriority = 1;

		switch(type) {
		case PWM_FILTER_DISABLE: //아직 구현 안함, 무조건 필터적용 되도록 되어있음
			break;
		case PWM_FILTER_ENABLE:
			break;
		}

		extiInit(extiDevice, &extiInitStructure, pwmHandler);
	}
}

#define LPF_FACTOR 1 //0.4

/*
 * @brief 수신기 읽기
 * @param data: 5개의 데이터 반환을 위한 포인터
 * @retval 없음
 */
void pwmRead(uint16_t *data) {
	uint8_t i = 0;
	static uint16_t preRcData[5] = { 0, }; //이전 데이터 저장을 위한 변수선언
	for(;i < RC_CHANNEL_MAX;i++) { //THR~AUX1
		if(rcDataVaild[i] == true) { //데이터가 유효함
			rcDataVaild[i] = false; //데이터 읽음을 표시
			if(preRcData[i] != 0) { //함수가 처음 호출되지 않았을 경우
				data[i] = LPF_FACTOR * rcRawData[i] + (1 - LPF_FACTOR) * preRcData[i]; //저주파 통과 필터(Lpf)
			}
			else { //함수가 처음 호출된경우
				data[i] = rcRawData[i];
			}
			preRcData[i] = rcRawData[i]; //현재 데이터 저장
		}
		else {
			data[i] = preRcData[i];
		}
	}
}

/*
 * @brief 수신기 인터럽트 핸들러
 * @note 외부인터럽트 핸들러에서 호출될 핸들러
 * @param extiDevice:
 * @retval 없음
 */
void pwmHandler(extiDevice_t extiDevice){
	if(GPIO_ReadInputDataBit(extiHardwareMap[extiChannel[extiDevice]].gpio, extiHardwareMap[extiChannel[extiDevice]].pin) == SET) { //rising 상태일때
		rcRising[extiDevice] = micros(); //현재의  micro초를 저장
	}
	else { //falling 상태일떄
		rcFalling[extiDevice] = micros();
		uint16_t buf = rcFalling[extiDevice] - rcRising[extiDevice]; //falling 와 rising시간을 빼서 pulse 시간 계산
		if(buf > RC_MIN && buf < RC_MAX) { //계산된 값이 최소값과 최대값 안의 값일때, 만약 이값을 벗어 날경우에 값이 반영되지 않도록 해둠
			rcRawData[extiDevice] = buf;
		}
		else {
			(buf <= RC_MIN) ? (rcRawData[extiDevice] = RC_MIN) : ((buf >= RC_MAX) ? (rcRawData[extiDevice] = RC_MAX) : (0));
		}
		rcDataVaild[extiDevice] = true; //데이터가 유효하다
	}
}
