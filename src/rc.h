#ifndef _RC_H_
#define _RC_H_

/*
 * @brief 수신기 장치 열거체
 */
typedef enum {
    RC_1 = 0,
    RC_2,
    MAX_RC,
} rcDevice_t;

/*
 * @brief 수신기 채널 열거체
 */
typedef enum {
	THR,
	AIL,
	ELE,
	RUD,
	AUX1,
	RC_CHANNEL_MAX,
} rcChannel_t;

/*
 * @brief 수신기 타입 열거체
 */
typedef enum {
	PWM = 0,
	PPM,
	S_BUS,
} rcType_t;

void rcInit(rcDevice_t rcDevice, rcType_t type);
void rcRead(uint16_t *data);

#endif
