#include "button.h"

/* Chân User Button (PC13) */
#define USER_BUTTON_PIN    13U

/* Hàm delay thô chống dội (Software debounce) */
static void Button_Delay(volatile uint32_t time)
{
    while (time--)
    {
    }
}

/* Khởi tạo GPIO cho nút nhấn (PC13) */
void Button_Init(void)
{
    /* Enable clock GPIOC */
    RCC->AHB1ENR |= (1U << 2);

    /* Cấu hình PC13 Mode: Input (00) */
    GPIOC->MODER &= ~(3U << (USER_BUTTON_PIN * 2U));

    /* Không dùng trở kéo nội (Pull-up đã có sẵn trên mạch Nucleo) */
    GPIOC->PUPDR &= ~(3U << (USER_BUTTON_PIN * 2U));
}

/* Đọc trạng thái vật lý (Raw state) của nút */
uint8_t Button_Read(void)
{
    /* Đọc giá trị tại bit 13 của thanh ghi IDR */
    uint8_t status = (GPIOC->IDR >> USER_BUTTON_PIN) & 0x1U;
    return status;
}

/* Phát hiện sự kiện nhấn-thả (Edge detection / Debounced) */
uint8_t Button_IsPressedEvent(void)
{
    static uint8_t prev = BUTTON_RELEASED;
    uint8_t current = Button_Read();

    /* Bắt sự kiện chuyển trạng thái từ PRESSED sang RELEASED */
    if ((prev == BUTTON_PRESSED) && (current == BUTTON_RELEASED))
    {
        /* Bỏ qua nhiễu tiếp điểm */
        Button_Delay(30000);

        current = Button_Read();

        /* Xác nhận trạng thái đã ổn định */
        if (current == BUTTON_RELEASED)
        {
            prev = current;
            return 1U;
        }
    }

    prev = current;
    return 0U;
}
