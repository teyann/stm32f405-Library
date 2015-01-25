#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_i2c.h>
#include <system.h>

/*
 *  -참조	 "https://code.google.com/p/afrodevices/wiki/AfroFlight"
 */

static void i2cErHandler(i2cDevice_t i2cDevice);
static void i2cEvHandler(i2cDevice_t i2cDevice);
static void i2cUnstick(i2cDevice_t i2cDevice);

/*
 * @brief i2c 에러카운터
 */
static volatile uint16_t i2cErrorCount = 0;

/*
 * @brief 에러 여부
 */
static volatile int8_t error = false;
/*
 * @brief i2c버스가 동작중일때
 */
static volatile int8_t busy;

/*
 * @brief 현재 i2c버스에서 주고받는 데이터
 */
static volatile uint8_t addr; // 연결된 장치 주소
static volatile uint8_t reg; // 레지스터 주소
static volatile uint8_t bytes; // 쓰거나 읽을 바이트 수
static volatile uint8_t writing; // 쓰는모드
static volatile uint8_t reading; // 읽는모드
static volatile uint8_t* writePtr; // 불러와서 쓸 데이터 포인터
static volatile uint8_t* readPtr; // 읽어서 저장할 데이터 포인터

/*
 * @brief i2c 초기화 구조체 초기설정
 * @param i2cInitStruct: 초기설정할 i2c 초기화 구조체 포인터
 * @retval 없음
 */
void i2cStructInit(i2cInitTypeDef_t* i2cInitStruct) {
	i2cInitStruct->clockSpeed = 400000;
    i2cInitStruct->preemptionPriority = 0;
    i2cInitStruct->subPriority = 0;
}

/*
 * @brief i2c 하드웨어 에러 핸들러
 * @param i2cDevice: i2c 장치 열거체
 * @retval 없음
 */
static ErrorStatus i2cHandleHardwareFailure(i2cDevice_t i2cDevice)
{
    i2cErrorCount++;
    // reinit peripheral + clock out garbage
    i2cInitTypeDef_t i2cInitStructure;
    i2cStructInit(&i2cInitStructure);
    i2cInit(i2cDevice, &i2cInitStructure);
    return ERROR;
}

/*
 * @brief i2c 버퍼 쓰기
 * @param i2cDevice: i2c 장치 열거체
 * @param addr_: 연결된 장치 주소
 * @param reg_: 레지스터 주소
 * @param len_: 데이터 바이트 수
 * @param data: 쓸 데이터의 포인터
 * @retval 에러여부(ERROR, SUCCESS)
 */
ErrorStatus i2cWriteBuffer(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t len_, uint8_t *data)
{
    uint32_t timeout;
    I2C_TypeDef *I2Cx = i2cHardwareMap[i2cDevice].i2c;

    addr = addr_ << 1;
    reg = reg_;
    bytes = len_;

    writing = true;
    reading = false;
    writePtr = data;
    readPtr = data;

    busy = true;
    error = false;

    if (!(I2Cx->CR2 & I2C_IT_EVT)) {                                    // if we are restarting the driver
        if (!(I2Cx->CR1 & 0x0100)) {                                    // ensure sending a start
        	timeout = I2C_DEFAULT_TIMEOUT;
            while (I2Cx->CR1 & 0x0200 && --timeout > 0);           // wait for any stop to finish sending
            if (timeout == 0) {
                return i2cHandleHardwareFailure(i2cDevice);
            }
            I2C_GenerateSTART(I2Cx, ENABLE);                            // send the start for the new job
        }
        I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, ENABLE);            // allow the interrupts to fire off again
    }

    timeout = I2C_DEFAULT_TIMEOUT;
    while (busy && --timeout > 0);
    if (timeout == 0) {
        return i2cHandleHardwareFailure(i2cDevice);
    }

    return !ERROR;
}

/*
 * @brief i2c 1바이트 쓰기
 * @param i2cDevice: i2c 장치 열거체
 * @param addr_: 연결된 장치 주소
 * @param reg_: 레지스터 주소
 * @param data: 쓸 데이터 포인터
 * @retval 에러여부(ERROR, SUCCESS)
 */
ErrorStatus i2cWrite(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t data)
{
    return i2cWriteBuffer(i2cDevice, addr_, reg_, 1, &data);
}

/*
 * @brief i2c 읽기
 * @param i2cDevice: i2c 장치 열거체
 * @param addr_: 연결된 장치 주소
 * @param reg_: 레지스터 주소
 * @param len_: 데이터 바이트 수
 * @param buf: 읽어서 저장할 데이터 포인터
 * @retval 에러여부(ERROR, SUCCESS)
 */
ErrorStatus i2cRead(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t len_, uint8_t* buf)
{
    uint32_t timeout;
    I2C_TypeDef *I2Cx = i2cHardwareMap[i2cDevice].i2c;

    addr = addr_ << 1;
    reg = reg_;
    bytes = len_;

    writing = false;
    reading = true;
    readPtr = buf;
    writePtr = buf;

    busy = true;
    error = false;

    if (!(I2Cx->CR2 & I2C_IT_EVT)) {                                    // if we are restarting the driver
        if (!(I2Cx->CR1 & 0x0100)) {                                    // ensure sending a start
        	timeout = I2C_DEFAULT_TIMEOUT;
            while (I2Cx->CR1 & 0x0200 && --timeout > 0);	            // wait for any stop to finish sending
            if (timeout == 0) {
                return i2cHandleHardwareFailure(i2cDevice);
            }
            I2C_GenerateSTART(I2Cx, ENABLE);                            // send the start for the new job
        }
        I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, ENABLE);            // allow the interrupts to fire off again
    }

    timeout = I2C_DEFAULT_TIMEOUT;
    while (busy && --timeout > 0);
    if (timeout == 0) {
        return i2cHandleHardwareFailure(i2cDevice);
    }

    return !ERROR;
}

/*
 * @brief i2c ER 핸들러
 * @param i2cDevice: i2c 장치 열거체
 * @retval 없음
 */
static void i2cErHandler(i2cDevice_t i2cDevice)
{
	I2C_TypeDef *I2Cx = i2cHardwareMap[i2cDevice].i2c;
    // Read the I2C1 status register
    volatile uint32_t SR1Reg = I2Cx->SR1;

    if (SR1Reg & 0x0F00)                                           // an error
        error = true;

    // If AF, BERR or ARLO, abandon the current job and commence new if there are jobs
    if (SR1Reg & 0x0700) {
        (void)I2Cx->SR2;                                                // read second status register to clear ADDR if it is set (note that BTF will not be set after a NACK)
        I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                        // disable the RXNE/TXE interrupt - prevent the ISR tailchaining onto the ER (hopefully)
        if (!(SR1Reg & I2C_FLAG_ARLO) && !(I2Cx->CR1 & 0x0200)) {         // if we dont have an ARLO error, ensure sending of a stop
            if (I2Cx->CR1 & 0x0100) {                                   // We are currently trying to send a start, this is very bad as start, stop will hang the peripheral
                while (I2Cx->CR1 & 0x0100) { ; }                        // wait for any start to finish sending
                I2C_GenerateSTOP(I2Cx, ENABLE);                         // send stop to finalise bus transaction
                while (I2Cx->CR1 & 0x0200) { ; }                        // wait for stop to finish sending
                i2cHandleHardwareFailure(i2cDevice);                                 // reset and configure the hardware
            }
            else {
                I2C_GenerateSTOP(I2Cx, ENABLE);                         // stop to free up the bus
                I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);   // Disable EVT and ERR interrupts while bus inactive
            }
        }
    }
    I2Cx->SR1 &= ~0x0F00;                                               // reset all the error bits to clear the interrupt
    busy = false;
}

/*
 * @brief i2c EV 핸들러
 * @param i2cDevice: i2c 장치 열거체
 * @retval 없음
 */
static void i2cEvHandler(i2cDevice_t i2cDevice)
{
	I2C_TypeDef *I2Cx = i2cHardwareMap[i2cDevice].i2c;
	static uint8_t subaddress_sent;                         // flag to indicate if subaddess sent, flag to indicate final bus condition
	static int8_t index;                                                // index is signed -1 == send the subaddress
	uint8_t SR1Reg = I2Cx->SR1;                                         // read the status register here

	if (SR1Reg & I2C_FLAG_SB) {                                              // we just sent a start - EV5 in ref manual
		I2Cx->CR1 &= ~0x0800;                                           // reset the POS bit so ACK/NACK applied to the current byte
		I2C_AcknowledgeConfig(I2Cx, ENABLE);                            // make sure ACK is on
		index = 0;                                                      // reset the index
		if (reading && subaddress_sent) {              // we have sent the subaddr
			subaddress_sent = 1;                                        // make sure this is set in case of no subaddress, so following code runs correctly
			if (bytes == 2) {
				I2Cx->CR1 |= 0x0800;                                    // set the POS bit so NACK applied to the final byte in the two byte read
			}
			I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Receiver);    // send the address and set hardware mode
		}
		else {                                                        // direction is Tx, or we havent sent the sub and rep start
			I2C_Send7bitAddress(I2Cx, addr, I2C_Direction_Transmitter); // send the address and set hardware mode
			index = -1;                                             // send a subaddress
		}
	}
	else if (SR1Reg & I2C_FLAG_ADDR) {                                       // we just sent the address - EV6 in ref manual
		// Read SR1,2 to clear ADDR                                                      // memory fence to control hardware
		__DMB();
		if (bytes == 1 && reading && subaddress_sent) {                 // we are receiving 1 byte - EV6_3
			I2C_AcknowledgeConfig(I2Cx, DISABLE);                       // turn off ACK
			__DMB();
			(void)I2Cx->SR2;                                            // clear ADDR after ACK is turned off
			I2C_GenerateSTOP(I2Cx, ENABLE);                             // program the stop
			I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);                     // allow us to have an EV7
		}
		else {                                                        // EV6 and EV6_1
			(void)I2Cx->SR2;                                            // clear the ADDR here
			__DMB();
			if (bytes == 2 && reading && subaddress_sent) {             // rx 2 bytes - EV6_1
				I2C_AcknowledgeConfig(I2Cx, DISABLE);                   // turn off ACK
				I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                // disable TXE to allow the buffer to fill
			}
			else if (bytes == 3 && reading && subaddress_sent) {        // rx 3 bytes
				I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                // make sure RXNE disabled so we get a BTF in two bytes time
			}
			else {                                                       // receiving greater than three bytes, sending subaddress, or transmitting
				I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);
			}
		}
	}
	else if (SR1Reg & I2C_FLAG_BTF) {                                        // Byte transfer finished - EV7_2, EV7_3 or EV8_2
		if (reading && subaddress_sent) {                               // EV7_2, EV7_3
			if (bytes > 2) {                                            // EV7_2
				I2C_AcknowledgeConfig(I2Cx, DISABLE);                   // turn off ACK
				readPtr[index++] = (uint8_t)I2Cx->DR;                    // read data N-2
				I2C_GenerateSTOP(I2Cx, ENABLE);                         // program the Stop
				readPtr[index++] = (uint8_t)I2Cx->DR;                    // read data N - 1
				I2C_ITConfig(I2Cx, I2C_IT_BUF, ENABLE);                 // enable TXE to allow the final EV7
			}
			else {                                                    // EV7_3
				I2C_GenerateSTOP(I2Cx, ENABLE);                     // program the Stop
				readPtr[index++] = (uint8_t)I2Cx->DR;                    // read data N - 1
				readPtr[index++] = (uint8_t)I2Cx->DR;                    // read data N
				index++;                                                // to show job completed
			}
		}
		else {                                                        // EV8_2, which may be due to a subaddress sent or a write completion
			if (subaddress_sent || writing) {
				I2C_GenerateSTOP(I2Cx, ENABLE);                     // program the Stop
				index++;                                                // to show that the job is complete
			}
			else {                                                    // We need to send a subaddress
				I2C_GenerateSTART(I2Cx, ENABLE);                        // program the repeated Start
				subaddress_sent = 1;                                    // this is set back to zero upon completion of the current task
			}
		}
		// we must wait for the start to clear, otherwise we get constant BTF
		while (I2Cx->CR1 & 0x0100);
	}
	else if (SR1Reg & I2C_FLAG_RXNE) {                                       // Byte received - EV7
		readPtr[index++] = (uint8_t)I2Cx->DR;
		if (bytes == (index + 3)) {
			I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                    // disable TXE to allow the buffer to flush so we can get an EV7_2
		}
		if (bytes == index) {                                             // We have completed a final EV7
			index++;                                                    // to show job is complete
		}
	}
	else if (SR1Reg & I2C_FLAG_TXE) {                                       // Byte transmitted EV8 / EV8_1
		if (index != -1) {                                              // we dont have a subaddress to send
			I2Cx->DR = writePtr[index++];
			if (bytes == index) {                                       // we have sent all the data
				I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                // disable TXE to allow the buffer to flush
			}
		}
		else {
			index++;
			I2Cx->DR = reg;                                             // send the subaddress
			if (reading || !bytes) {                                     // if receiving or sending 0 bytes, flush now
				I2C_ITConfig(I2Cx, I2C_IT_BUF, DISABLE);                // disable TXE to allow the buffer to flush
			}
		}
	}
	if (index == bytes + 1) {                                           // we have completed the current job
		subaddress_sent = 0;                                            // reset this here
		I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);       // Disable EVT and ERR interrupts while bus inactive
		busy = false;
	}
}

/*
 * @brief i2c 초기화
 * @param i2cDevice: i2c 장치 열거체
 * @param i2cInitStruct: i2c 초기화시 기본 설정 구조체 포인터
 * @retval 없음
 */
void i2cInit(i2cDevice_t i2cDevice, i2cInitTypeDef_t* i2cInitStruct)
{
    I2C_TypeDef *I2Cx = i2cHardwareMap[i2cDevice].i2c;

    RCC_AHB1PeriphClockCmd(i2cHardwareMap[i2cDevice].gpioPeriph, ENABLE);
    RCC_APB1PeriphClockCmd(i2cHardwareMap[i2cDevice].i2cPeriph, ENABLE);

    i2cUnstick(i2cDevice);

    I2C_DeInit(I2Cx);

    GPIO_PinAFConfig(i2cHardwareMap[i2cDevice].gpio, 6, GPIO_AF_I2C1);
    GPIO_PinAFConfig(i2cHardwareMap[i2cDevice].gpio, 7, GPIO_AF_I2C1);

    I2C_InitTypeDef I2C_InitStructure;
    I2C_StructInit(&I2C_InitStructure);
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = i2cInitStruct->clockSpeed; //400000
    I2C_Init(I2Cx, &I2C_InitStructure);

    I2C_ITConfig(I2Cx, I2C_IT_EVT | I2C_IT_ERR, DISABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = i2cHardwareMap[i2cDevice].erIrq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = i2cInitStruct->preemptionPriority; //0
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = i2cInitStruct->subPriority; //0
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = i2cHardwareMap[i2cDevice].evIrq;
    NVIC_Init(&NVIC_InitStructure);

    I2C_Cmd(I2Cx, ENABLE);
}

/*
 * @brief i2c 에러 카운터 읽기
 * @param 없음
 * @retval i2cErrorCount(uint16_t)
 */
uint16_t i2cGetErrorCounter(void)
{
    return i2cErrorCount;
}

/*
 * @brief i2c 딜레이
 * @param 없음
 * @retval 없음
 */
static void i2cDelay(void) {
	delayMicroseconds(10);
}

/*
 * @brief i2c 통신 초기화
 * @note 초기 연결시에 발생하는 노이즈로 인한 통신 에러를 방지하고, 데이터 통신중 에러가 발생했을때 호출
 * @param i2cDevice: i2c 장치 열거체
 * @retval 없음
 */
static void i2cUnstick(i2cDevice_t i2cDevice)
{
    GPIO_TypeDef *gpio;
    uint16_t scl, sda;

    // prepare pins
    gpio = i2cHardwareMap[i2cDevice].gpio;
    scl = i2cHardwareMap[i2cDevice].scl;
    sda = i2cHardwareMap[i2cDevice].sda;

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = scl | sda; //outod
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(gpio, &GPIO_InitStructure);

    digitalHi(gpio, scl | sda);

    uint8_t i = 0;
    for (i = 0; i < 8; i++) {
        // Wait for any clock stretching to finish
        while (!digitalIn(gpio, scl)) {
            i2cDelay();
        }
        // Pull low
        digitalLo(gpio, scl); // Set bus low
        i2cDelay();
        // Release high again
        digitalHi(gpio, scl); // Set bus high
        i2cDelay();
    }

    // Generate a start then stop condition
    // SCL  PB10
    // SDA  PB11
    digitalLo(gpio, sda); // Set bus data low
    i2cDelay();
    digitalLo(gpio, scl); // Set bus scl low
    i2cDelay();
    digitalHi(gpio, scl); // Set bus scl high
    i2cDelay();
    digitalHi(gpio, sda); // Set bus sda high

    // Init pins
    GPIO_InitStructure.GPIO_Pin = scl | sda; //afod
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(gpio, &GPIO_InitStructure);
}

void I2C1_ER_IRQHandler(void)
{
    i2cErHandler(I2C_DEVICE_1);
}

void I2C1_EV_IRQHandler(void)
{
    i2cEvHandler(I2C_DEVICE_1);
}

void I2C2_ER_IRQHandler(void)
{
    i2cErHandler(I2C_DEVICE_2);
}

void I2C2_EV_IRQHandler(void)
{
    i2cEvHandler(I2C_DEVICE_2);
}
