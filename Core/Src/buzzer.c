#include "buzzer.h"

/* Trạng thái hiện tại của còi (0: Tắt, 1: Bật) */
static uint8_t buzzer_state = 0;

/* Lưu mốc thời gian chuyển trạng thái để tạo nhịp (ms) */
static uint32_t buzzer_last_change_time = 0;

/* Khởi tạo GPIO cho Buzzer (PC3) */
void Buzzer_Init(void)
{
    /* Enable clock GPIOC */
    RCC->AHB1ENR |= (1 << 2);

    /* Cấu hình PC3 Mode: Output (01) */
    BUZZER_GPIO_PORT->MODER &= ~(3 << (BUZZER_PIN * 2));
    BUZZER_GPIO_PORT->MODER |=  (1 << (BUZZER_PIN * 2));

    /* Cấu hình Output Type: Push-Pull */
    BUZZER_GPIO_PORT->OTYPER &= ~(1 << BUZZER_PIN);

    /* Cấu hình Output Speed: Low */
    BUZZER_GPIO_PORT->OSPEEDR &= ~(3 << (BUZZER_PIN * 2));

    /* Không dùng điện trở kéo (No Pull-up/Pull-down) */
    BUZZER_GPIO_PORT->PUPDR &= ~(3 << (BUZZER_PIN * 2));

    /* Khởi tạo trạng thái ban đầu: Tắt */
    Buzzer_Off();
    buzzer_state = 0;
    buzzer_last_change_time = HAL_GetTick();
}

/* Bật Buzzer (Active-High) */
void Buzzer_On(void)
{
    /* Set bit PC3 */
    BUZZER_GPIO_PORT->ODR |= (1 << BUZZER_PIN);
    buzzer_state = 1;
}

/* Tắt Buzzer */
void Buzzer_Off(void)
{
    /* Clear bit PC3 */
    BUZZER_GPIO_PORT->ODR &= ~(1 << BUZZER_PIN);
    buzzer_state = 0;
}

/* Đảo trạng thái Buzzer (Toggle) */
void Buzzer_Toggle(void)
{
    if (buzzer_state)
    {
        Buzzer_Off();
    }
    else
    {
        Buzzer_On();
    }
}

/* Cập nhật nhịp kêu của Buzzer theo khoảng cách (Non-blocking) */
void Buzzer_UpdateByDistance(float d_measured)
{
    uint32_t now = HAL_GetTick();

    /* Tắt Buzzer nếu đo lỗi hoặc ngoài vùng nguy hiểm */
    if ((d_measured < 0.0f) || (d_measured >= BUZZER_DANGER_DISTANCE_CM))
    {
        Buzzer_Off();
        buzzer_last_change_time = now; // Reset mốc thời gian để nhịp sau kêu đều
        return;
    }

    /* Trong vùng nguy hiểm: Xử lý nhịp kêu (Beep) */
    if (buzzer_state)
    {
        /* Chuyển sang trạng thái Tắt nếu hết thời gian On */
        if ((now - buzzer_last_change_time) >= BUZZER_BEEP_ON_TIME_MS)
        {
            Buzzer_Off();
            buzzer_last_change_time = now;
        }
    }
    else
    {
        /* Chuyển sang trạng thái Bật nếu hết thời gian Off */
        if ((now - buzzer_last_change_time) >= BUZZER_BEEP_OFF_TIME_MS)
        {
            Buzzer_On();
            buzzer_last_change_time = now;
        }
    }
}
