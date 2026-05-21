#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"
#include <stdint.h>

void Motor_Init(void);
// speedLeft, speedRight điền từ 0 đến 100
void Motor_Forward(uint8_t speedLeft, uint8_t speedRight);
void Motor_Stop(void);

#endif
