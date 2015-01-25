#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_uart.h>

static void serialInit(uartDevice_t uartDevice_);
void setSystemClock(uint32_t clocks);
uint32_t getSystemClock(void);

/*
 * @brief 시스템 클럭수가 저장될 변수
 */
static uint32_t systemClocks = 0;

/*
 * @brief 시스템 tick타이머 카운터가 저장될 함수, 밀리초
 * @note !경고, uint32_t 데이터 타입의 범위가 유한하므로 50일이 지나면 0으로 돌아감
 */
static volatile uint32_t sysTickNum;

/*
 * @brief 사용할 외부인터럽트 장치를 저장할 포인터
 */
static uint32_t usTicks;

/*
 * @brief 사용할 외부인터럽트 장치를 저장할 포인터
 */
static uartDevice_t uartDevice = 0;

/*
 * @brief 시스템 초기화
 * @param 없음
 * @retval 없음
 */
void systemInit(void) {
	SystemInit();
	RCC_ClocksTypeDef rcc_clocks;
	RCC_GetClocksFreq(&rcc_clocks);
	uint32_t systemClocks_ = rcc_clocks.SYSCLK_Frequency;
	setSystemClock(systemClocks_);
	//72000000 / 1000000 = 72
	usTicks = systemClocks_ / 1000000;
	//72000000 / 1000 = 72000, sysTick init
	SysTick_Config(systemClocks_ / 1000);

	serialInit(UART_DEVICE_6);
}

/*
 * @brief 시리얼 초기화
 * @param uartDevice_: 유아트 장치 열거체
 * @retval 없음
 */
static void serialInit(uartDevice_t uartDevice_) {
	uartDevice = uartDevice_;
	uartInitTypeDef_t uartInitStructure;
	uartInitStructure.preemptionPriority = 0;
	uartInitStructure.subPriority = 1;
	uartInitStructure.baudRate = 115200;
	uartInit(uartDevice_, &uartInitStructure);
}

/*
 * @brief 시리얼  문자 쓰기
 * @param c: 쓸 데이터
 * @retval 없음
 */
void serialPutChar(uint8_t c) {
	uartPutChar(uartDevice, c);
}

/*
 * @brief 시리얼 문자 읽기
 * @param 없음
 * @retval 읽은 데이터(uint8_t)
 */
uint8_t serialGetChar(void) {
	return uartGetChar(uartDevice);
}

/*
 * @brief 시스템 클럭 저장
 * @param clocks: 저장할 클럭수
 * @retval 없음
 */
void setSystemClock(uint32_t clocks) {
	systemClocks = clocks;
}

/*
 * @brief 시스템 클럭 읽기
 * @param 없음
 * @retval 시스템 클럭(uint32_t)
 */
uint32_t getSystemClock(void) {
	return systemClocks;
}

/*
 * @brief 현재 마이크로초 읽기
 * @note !경고, uint32_t 데이터 타입의 범위가 유한하므로 70분이 지나면 0으로 돌아감
 * @param 없음
 * @retval 시스템 초기화후 지난 마이크로초(uint32_t)
 */
uint32_t micros(void)
{
    register uint32_t ms, cycle_cnt;
    do {
        ms = sysTickNum;
        cycle_cnt = SysTick->VAL;
    } while (ms != sysTickNum); // SysTick->VAL가 유효할때
    // 밀리초는 마이크로초의 10^3배이므로 *1000
    // 시스템  tick의 val값은 설정했던 분주비를 바탕으로 업데이트 되고, 기본 클럭이 72Mhz이고 설정을 1Mhz로 했으므로 / 72
    return (ms * 1000) + ((usTicks * 1000 - cycle_cnt) / usTicks);
}

/*
 * @brief 현재 밀리초읽기
 * @note !경고, uint32_t 데이터 타입의 범위가 유한하므로 50일이 지나면 0으로 돌아감
 * @param 없음
 * @retval 시스템 초기화후 지난 밀리초(uint32_t)
 */
uint32_t millis(void)
{
	// sysTickUptime(ms)
    return sysTickNum;
}

/*
 * @brief 마이크로초 딜레이
 * @param us: 딜레이 할 마이크로초
 * @retval 없음
 */
void delayMicroseconds(uint32_t us)
{
    uint32_t now = micros();
    while (micros() - now < us);
}

/*
 * @brief 밀리초 딜레이
 * @param ms: 딜레이 할 밀리초
 * @retval 없음
 */
void delay(uint32_t ms)
{
    while (ms--) {
    	// 1ms = 1000us
        delayMicroseconds(1000);
    }
}

void SysTick_Handler() {
	sysTickNum++;
}
