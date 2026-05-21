#include "lcd.h"
#include "i2c.h"
#include <stdio.h> // Hỗ trợ hàm snprintf()

/* Handle I2C1 được sinh ra từ CubeMX */
extern I2C_HandleTypeDef hi2c1;

/* Các bit mask điều khiển qua module I2C PCF8574 */
#define LCD_RS          0x01 // Register Select (0: Lệnh, 1: Ký tự)
#define LCD_RW          0x02 // Read/Write (Mặc định 0: Ghi)
#define LCD_EN          0x04 // Enable (Chân tạo xung chốt)
#define LCD_BL          0x08 // Backlight (Đèn nền)

/* Gửi 1 byte dữ liệu vật lý qua bus I2C (Timeout 100ms) */
static void LCD_WriteI2C(uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &data, 1, 100);
}

/* Tạo xung chốt (Enable Pulse) để LCD đọc dữ liệu */
static void LCD_PulseEnable(uint8_t data)
{
    LCD_WriteI2C(data | LCD_EN | LCD_BL);  // Cạnh lên
    HAL_Delay(1);
    LCD_WriteI2C((data & ~LCD_EN) | LCD_BL); // Cạnh xuống (Chốt data)
    HAL_Delay(1);
}

/* Gửi 4 bit cao của dữ liệu tới LCD (Chế độ 4-bit) */
static void LCD_Send4Bits(uint8_t data, uint8_t mode)
{
    uint8_t data_to_send = (data & 0xF0) | mode | LCD_BL;
    LCD_PulseEnable(data_to_send);
}

/* Tách và gửi 1 byte hoàn chỉnh (Gửi 4 bit cao trước, 4 bit thấp sau) */
static void LCD_SendByte(uint8_t data, uint8_t mode)
{
    LCD_Send4Bits(data & 0xF0, mode);           // 4 bit cao
    LCD_Send4Bits((data << 4) & 0xF0, mode);    // 4 bit thấp
}

/* Gửi byte lệnh (Command - RS = 0) */
static void LCD_SendCommand(uint8_t cmd)
{
    LCD_SendByte(cmd, 0);
    HAL_Delay(2);
}

/* Khởi tạo module LCD I2C */
void LCD_Init(void)
{
    /* Đợi LCD ổn định nguồn */
    HAL_Delay(50);

    /* Trình tự khởi tạo bắt buộc của chip HD44780 */
    LCD_Send4Bits(0x30, 0);
    HAL_Delay(5);
    LCD_Send4Bits(0x30, 0);
    HAL_Delay(1);
    LCD_Send4Bits(0x30, 0);
    HAL_Delay(1);

    /* Chuyển sang chế độ 4-bit */
    LCD_Send4Bits(0x20, 0);
    HAL_Delay(1);

    /* Function Set: 4-bit mode, 2 dòng, font 5x8 */
    LCD_SendCommand(0x28);

    /* Display Control: Bật màn hình, tắt con trỏ, tắt nhấp nháy */
    LCD_SendCommand(0x0C);

    /* Entry Mode: Con trỏ tự dịch phải sau khi ghi */
    LCD_SendCommand(0x06);

    /* Xóa màn hình */
    LCD_Clear();
}

/* Xóa trắng toàn bộ màn hình */
void LCD_Clear(void)
{
    LCD_SendCommand(0x01);
    HAL_Delay(2); // Lệnh Clear cần thời gian xử lý lâu hơn
}

/* Đưa con trỏ tới tọa độ (Row, Col) */
void LCD_SetCursor(uint8_t row, uint8_t col)
{
    uint8_t row_offsets[] = {0x00, 0x40}; // Địa chỉ DDRAM bắt đầu của 2 dòng

    if (row >= LCD_ROWS) row = 0;
    if (col >= LCD_COLS) col = 0;

    LCD_SendCommand(0x80 | (row_offsets[row] + col));
}

/* In 1 ký tự ra màn hình (Data - RS = 1) */
void LCD_PrintChar(char data)
{
    LCD_SendByte((uint8_t)data, LCD_RS);
}

/* In chuỗi ký tự (Kết thúc bằng '\0') */
void LCD_PrintString(const char *str)
{
    while (*str)
    {
        LCD_PrintChar(*str);
        str++;
    }
}

/* Format và hiển thị thông số ADAS (Khoảng cách & Trạng thái) */
void LCD_DisplayADAS(float distance_cm, const char *state)
{
    char line1[17];
    char line2[17];

    /* Xử lý hiển thị dòng 1 (Khoảng cách) */
    if (distance_cm >= 0.0f)
    {
        snprintf(line1, sizeof(line1), "Dist:%4d cm", (int)distance_cm);
    }
    else
    {
        snprintf(line1, sizeof(line1), "Dist:No Echo");
    }

    /* Xử lý hiển thị dòng 2 (Trạng thái) */
    snprintf(line2, sizeof(line2), "State:%s", state);

    /* Xóa và in lại toàn bộ */
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_PrintString(line1);

    LCD_SetCursor(1, 0);
    LCD_PrintString(line2);
}
