#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <drv_i2c.h>
#include <mpu6050.h>
#include <system.h>

/*
 * @brief 사용할 i2c 장치를 저장할 변수
 */
static i2cDevice_t i2cDevice = 0;

/*
 * @brief mpu6050 초기화
 * @note 내부 lpf 설정 부분 아직 구현안함
 * @param i2cDevice_: i2c 장치 열거체
 * @retval 없음
 */
void mpu6050Init(i2cDevice_t i2cDevice_) {
	i2cDevice = i2cDevice_;
	i2cInitTypeDef_t i2cInitStructure;
	i2cStructInit(&i2cInitStructure);
	i2cInit(i2cDevice, &i2cInitStructure);
	i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x80);      //PWR_MGMT_1    -- DEVICE_RESET 1
	delay(5);
	i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_SMPLRT_DIV, 0x00);      //SMPLRT_DIV    -- SMPLRT_DIV = 0  Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
	i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x03);      //PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference)
	/*i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0);*/
	/*i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_CONFIG, mpuLowPassFilter);*/
	i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
	i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_ACCEL_CONFIG, INV_FSR_8G << 3);
	delay(5);
}

/*
 * @brief mpu6050 읽기
 * @note 함수가 상당히 복잡함, 간단하게 정리가 필요함
 * @param type: mpu6050에서 읽을 데이터 열거체
 * @param data: 출력할 데이터 포인터
 * @retval error
 */
ErrorStatus mpu6050Read(mpu6050Type_t type, int16_t* data) {
	uint8_t buf8[6];
	uint8_t reg;

	switch(type) {
		case ACC:
			reg = MPU_RA_ACCEL_XOUT_H;
			break;
		case GYRO:
			reg = MPU_RA_GYRO_XOUT_H;
			break;
	}
	static bool errorFlag = 0, flag1 = 1, flag2 = 0, flag3 = 0;
	static uint32_t startTime = 0;

	if(errorFlag != 0) {
		uint16_t timer = millis() - startTime;
		if(flag1 == 1 && timer > 0) {
			i2cInitTypeDef_t i2cInitStructure;
			i2cStructInit(&i2cInitStructure);
			i2cInit(i2cDevice, &i2cInitStructure);
			i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x80);
			startTime = millis();
			flag2 = 1; flag1 = 0;
		}
		timer = millis() - startTime;
		if(flag2 == 1 && timer > 3) {
			i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_SMPLRT_DIV, 0x00);      //SMPLRT_DIV    -- SMPLRT_DIV = 0  Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
			i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_PWR_MGMT_1, 0x03);      //PWR_MGMT_1    -- SLEEP 0; CYCLE 0; TEMP_DIS 0; CLKSEL 3 (PLL with Z Gyro reference)
			i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
			i2cWrite(i2cDevice, MPU6050_ADDRESS, MPU_RA_ACCEL_CONFIG, INV_FSR_8G << 3);
			startTime = millis();
			flag3 = 1; flag2 = 0;
		}
		timer = millis() - startTime;
		if(flag3 == 1 && timer > 5) {
			errorFlag = 0;
			startTime = 0;
			startTime = 0;
			flag1 = 1; flag3 = 0;
		}
		data[0] = 0;
		data[1] = 0;
		data[2] = 0;
		return ERROR;
	}

	i2cRead(i2cDevice, 0x68, reg, 6, buf8);

	static uint8_t preErrCounter = 0;
	uint8_t errCounter = i2cGetErrorCounter();
	if(errCounter != preErrCounter) {
		preErrCounter = errCounter;
		return ERROR;
	}

	uint16_t buf16[3];
	buf16[0] = ((buf8[0] << 8) | buf8[1]);
	buf16[1] = ((buf8[2] << 8) | buf8[3]);
	buf16[2] = ((buf8[4] << 8) | buf8[5]);

	if((buf16[0] == 0) && (buf16[1] == 0) && (buf16[2] == 0)) {
		errorFlag = 1;
		startTime = millis();
		return ERROR;
	}

	switch(type) {
		case ACC:
			ACC_ORIENTATION(data[0], data[1], data[2], buf16[0], buf16[1], buf16[2]);
			break;
		case GYRO:
			GYRO_ORIENTATION(data[0], data[1], data[2], buf16[0], buf16[1], buf16[2]);
			break;
	}
	return !ERROR;
}
