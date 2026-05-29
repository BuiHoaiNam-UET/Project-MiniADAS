#ifndef LED_H
#define LED_H

#include "main.h"

/* Cấu hình phần cứng cho LED cảnh báo (PC0) */
#define LED_GPIO_PORT GPIOC
#define LED_PIN 0

/* Khởi tạo GPIO Output cho LED */
void LED_Init(void);

/* Các hàm thao tác mức vật lý */
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

/* Cập nhật trạng thái LED theo ngưỡng khoảng cách (Bật khi < threshold hoặc lỗi đo) */
void LED_UpdateByDistance(float distance_cm, float threshold_cm);

#endif
