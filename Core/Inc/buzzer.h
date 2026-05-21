#ifndef BUZZER_H
#define BUZZER_H

#include "main.h"
#include <stdint.h>

/* Cấu hình phần cứng cho Buzzer (PC3) */
#define BUZZER_GPIO_PORT              GPIOC
#define BUZZER_PIN                    3U

/* Ngưỡng khoảng cách kích hoạt còi cảnh báo (cm) */
#define BUZZER_DANGER_DISTANCE_CM     10.0f

/* Chu kỳ nhịp On/Off của còi (ms) */
#define BUZZER_BEEP_ON_TIME_MS        100U
#define BUZZER_BEEP_OFF_TIME_MS       100U

/* Khởi tạo GPIO Output cho Buzzer */
void Buzzer_Init(void);

/* Các hàm điều khiển mức vật lý */
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Toggle(void);

/* Xử lý logic nhịp kêu của còi theo khoảng cách thực tế (Non-blocking) */
void Buzzer_UpdateByDistance(float d_measured);

#endif
