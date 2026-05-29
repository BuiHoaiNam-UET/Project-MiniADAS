#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"
#include <stdint.h>

/* User Button (PC13) - Active-low */
#define BUTTON_PRESSED 0
#define BUTTON_RELEASED 1

/* Khởi tạo GPIO cho nút nhấn PC13 */
void Button_Init(void);

/* Đọc trạng thái vật lý (Raw state) của nút */
uint8_t Button_Read(void);

/* Bắt sự kiện nhấn-thả (Edge detection/Debounce). Trả về 1 khi có sự kiện. */
uint8_t Button_IsPressedEvent(void);

#endif
