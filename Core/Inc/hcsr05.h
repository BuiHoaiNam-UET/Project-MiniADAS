#ifndef HCSR05_H
#define HCSR05_H

#include "main.h"
#include <stdint.h>

/* Tần số clock cấp cho TIM3 (MHz) để tính toán Prescaler (Tick = 1us) */
#define HCSR05_TIMER_CLK_MHZ        84U

/* Timeout chờ xung Echo (30000us tương đương ngưỡng >400cm) */
#define HCSR05_TIMEOUT_US           30000U

/* Giá trị trả về khi lỗi đo (Timeout, không có vật cản...) */
#define HCSR05_INVALID_DISTANCE     (-1.0f)

/* Khởi tạo Trig (PA7), Echo (PA6 - TIM3_CH1) và Timer Input Capture */
void HCSR05_Init(void);

/* Đọc khoảng cách (cm). Trả về HCSR05_INVALID_DISTANCE nếu lỗi */
float HCSR05_ReadDistanceCm(void);

/* Đọc độ rộng xung Echo (us) - Dùng để debug hoặc tính toán raw */
uint32_t HCSR05_ReadEchoPulseUs(void);

/* Hàm tạo trễ Microsecond bằng TIM3 (Phục vụ cấp xung Trig 10us) */
void HCSR05_DelayUs(uint32_t us);

#endif
