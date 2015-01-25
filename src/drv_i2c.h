#ifndef _I2C_H_
#define _I2C_H_

#ifndef bool
typedef uint8_t bool;
#define false (bool) 0
#define true (bool) 1
#define NULL ((void *)0)
#endif

/*
 * @brief i2c 장치 열거체
 */
typedef enum i2cChannel_s {
    I2C_DEVICE_1 = 0,
    I2C_DEVICE_2,
    MAX_I2C_DEVICE,
} i2cDevice_t;

/*
 * @brief i2c 하드웨어 맵핑을 위한 구조체
 */
typedef struct i2cDevice_s {
    I2C_TypeDef *i2c;
    GPIO_TypeDef *gpio;
    uint16_t scl;
    uint16_t sda;
    uint8_t evIrq;
    uint8_t erIrq;
    uint32_t gpioPeriph;
    uint32_t i2cPeriph;
} i2cHardwareMap_t;

/*
 * @brief 타이머 하드웨어 맵핑
 */
static const i2cHardwareMap_t i2cHardwareMap[] = {
    { I2C1, GPIOB, GPIO_Pin_6, GPIO_Pin_7, I2C1_EV_IRQn, I2C1_ER_IRQn, RCC_AHB1Periph_GPIOB, RCC_APB1Periph_I2C1 },
    { I2C2, GPIOB, GPIO_Pin_10, GPIO_Pin_11, I2C2_EV_IRQn, I2C2_ER_IRQn, RCC_AHB1Periph_GPIOB, RCC_APB1Periph_I2C2 },
};

/*
 * @brief i2c 초기화 타입 구조체
 */
typedef struct
{
	uint8_t preemptionPriority;
	uint8_t subPriority;
	uint32_t clockSpeed;
} i2cInitTypeDef_t;

#define I2C_DEFAULT_TIMEOUT 3000

void i2cInit(i2cDevice_t i2cDevice, i2cInitTypeDef_t* i2cInitStruct);
void i2cStructInit(i2cInitTypeDef_t* i2cInitStruct);
ErrorStatus i2cWriteBuffer(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t len_, uint8_t *data);
ErrorStatus i2cWrite(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t data);
ErrorStatus i2cRead(i2cDevice_t i2cDevice, uint8_t addr_, uint8_t reg_, uint8_t len_, uint8_t* buf);
uint16_t i2cGetErrorCounter(void);

#endif
