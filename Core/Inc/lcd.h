#ifndef LCD_H
#define LCD_H

#include "main.h"
#include <stdint.h>

/* Địa chỉ I2C của LCD 1602 (Dịch trái 1 bit cho HAL). Thử (0x3F << 1) nếu không nhận */
#define LCD_I2C_ADDR        (0x27 << 1)

/* Kích thước màn hình */
#define LCD_COLS            16
#define LCD_ROWS            2

/* Khởi tạo LCD (Yêu cầu phải gọi MX_I2C1_Init() trước) */
void LCD_Init(void);

/* Xóa trắng màn hình */
void LCD_Clear(void);

/* Đặt vị trí con trỏ (row: 0-1, col: 0-15) */
void LCD_SetCursor(uint8_t row, uint8_t col);

/* In 1 ký tự */
void LCD_PrintChar(char data);

/* In chuỗi ký tự */
void LCD_PrintString(const char *str);

/* Cập nhật giao diện ADAS (VD: Dòng 1 chứa Dist, Dòng 2 chứa State) */
void LCD_DisplayADAS(float distance_cm, const char *state);

#endif
