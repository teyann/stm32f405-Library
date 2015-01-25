#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_uart.h>

static volatile uartBuf_t uartBuf[MAX_UART_DEVICE];
static USART_TypeDef* uartPeriph[MAX_UART_DEVICE];

/*
 * @brief 유아트 초기화
 * @param uartDevice: 유아트 장치 열거체
 * @param uartInitStruct: 유아트 초기화를 위한 구조체 변수
 * @retval 없음
 */
void uartInit(uartDevice_t uartDevice, uartInitTypeDef_t* uartInitStruct) {
	uartPeriph[uartDevice] = uartHardwareMap[uartDevice].uart;

	if(uartDevice != UART_DEVICE_1 && uartDevice != UART_DEVICE_6) {
		RCC_APB1PeriphClockCmd(uartHardwareMap[uartDevice].uartClock, ENABLE);
	}
	else {
		RCC_APB2PeriphClockCmd(uartHardwareMap[uartDevice].uartClock, ENABLE);
	}

	RCC_AHB1PeriphClockCmd(uartHardwareMap[uartDevice].gpioClock, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = 1 << uartHardwareMap[uartDevice].txPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(uartHardwareMap[uartDevice].gpio, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = 1 << uartHardwareMap[uartDevice].rxPin;
	GPIO_Init(uartHardwareMap[uartDevice].gpio, &GPIO_InitStructure);

	GPIO_PinAFConfig(uartHardwareMap[uartDevice].gpio, uartHardwareMap[uartDevice].txPin, uartHardwareMap[uartDevice].gpioAF);
	GPIO_PinAFConfig(uartHardwareMap[uartDevice].gpio, uartHardwareMap[uartDevice].rxPin, uartHardwareMap[uartDevice].gpioAF);

	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = uartInitStruct->baudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(uartHardwareMap[uartDevice].uart, &USART_InitStructure);

	USART_ITConfig(uartHardwareMap[uartDevice].uart, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = uartHardwareMap[uartDevice].irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = uartInitStruct->preemptionPriority;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = uartInitStruct->subPriority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(uartHardwareMap[uartDevice].uart, ENABLE);
}

/*
 * @brief 유아트 문자 쓰기
 * @param uartDevice: 유아트 장치 열거체
 * @param c: 쓸 문자
 * @retval 없음
 */
void uartPutChar (uartDevice_t uartDevice, uint8_t c)
{
	USART_TypeDef* UARTx = uartPeriph[uartDevice];
	if( c == '\n')
	{
		uartBuf[uartDevice].TX.Buf[uartBuf[uartDevice].TX.BufHead++] = '\r';
		uartBuf[uartDevice].TX.BufHead %= BUFFER_SIZE;
	}
	uartBuf[uartDevice].TX.Buf[uartBuf[uartDevice].TX.BufHead++] = c;
	uartBuf[uartDevice].TX.BufHead %= BUFFER_SIZE;
	USART_ITConfig(UARTx, USART_IT_TXE, ENABLE);
}

/*
 * @brief 유아트 문자 읽기
 * @param uartDevice: 유아트 장치 열거체
 * @retval 읽은 데이터(uint8_t)
 */
uint8_t uartGetChar(uartDevice_t uartDevice)
{
	uint8_t buf;
	if(uartBuf[uartDevice].RX.BufHead == uartBuf[uartDevice].RX.BufTail)
	{
		buf = '?';
	}
	else
	{
		buf = uartBuf[uartDevice].RX.Buf[uartBuf[uartDevice].RX.BufTail++];
		uartBuf[uartDevice].RX.BufTail %= BUFFER_SIZE;
	}
	return buf;
}

/*
 * @brief 유아트 인터럽트 핸들러
 * @param uartDevice: 유아트 장치 열거체
 * @retval 없음
 */
void uartHandler(uartDevice_t uartDevice) {
	USART_TypeDef* UARTx = uartPeriph[uartDevice];
	if(USART_GetITStatus(UARTx, USART_IT_RXNE) != RESET)
	{
		uartBuf[uartDevice].RX.Buf[uartBuf[uartDevice].RX.BufHead++] = USART_ReceiveData(UARTx);
		uartBuf[uartDevice].RX.BufHead %= BUFFER_SIZE;
	}
	if(USART_GetITStatus(UARTx, USART_IT_TXE) != RESET)
	{
		USART_SendData(UARTx, uartBuf[uartDevice].TX.Buf[uartBuf[uartDevice].TX.BufTail++]);
		uartBuf[uartDevice].TX.BufTail %= BUFFER_SIZE;
		if(uartBuf[uartDevice].TX.BufHead == uartBuf[uartDevice].TX.BufTail)
		{
			USART_ITConfig(UARTx, USART_IT_TXE, DISABLE);
		}
	}
}

void USART1_IRQHandler(void)
{
	uartHandler(UART_DEVICE_1);
}

void USART2_IRQHandler(void)
{
	uartHandler(UART_DEVICE_2);
}

void USART3_IRQHandler(void)
{
	uartHandler(UART_DEVICE_3);
}

void UART4_IRQHandler(void)
{
	uartHandler(UART_DEVICE_4);

}

void UART5_IRQHandler(void)
{
	uartHandler(UART_DEVICE_5);
}


void USART6_IRQHandler(void)
{
	uartHandler(UART_DEVICE_6);
}
