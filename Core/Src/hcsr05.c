#include "hcsr05.h"

/* Cấu hình chân phần cứng */
#define HCSR05_TRIG_PIN             7U  // PA7: GPIO Output
#define HCSR05_ECHO_PIN             6U  // PA6: TIM3_CH1 (Input Capture)
#define HCSR05_TIM3_AF              2U  // Alternate Function 2 cho TIM3

/* Các hàm thao tác mức vật lý cho chân Trig */
static void HCSR05_TrigHigh(void)
{
    GPIOA->ODR |= (1U << HCSR05_TRIG_PIN);
}

static void HCSR05_TrigLow(void)
{
    GPIOA->ODR &= ~(1U << HCSR05_TRIG_PIN);
}

/* Cấu hình Input Capture bắt cạnh lên (Start of Echo) */
static void HCSR05_SetCaptureRisingEdge(void)
{
    TIM3->CCER &= ~(1U << 0);       // Disable Capture
    TIM3->CCER &= ~(1U << 1);       // CC1P = 0
    TIM3->CCER &= ~(1U << 3);       // CC1NP = 0 (Rising edge)
    TIM3->CCER |= (1U << 0);        // Enable Capture
}

/* Cấu hình Input Capture bắt cạnh xuống (End of Echo) */
static void HCSR05_SetCaptureFallingEdge(void)
{
    TIM3->CCER &= ~(1U << 0);       // Disable Capture
    TIM3->CCER |= (1U << 1);        // CC1P = 1
    TIM3->CCER &= ~(1U << 3);       // CC1NP = 0 (Falling edge)
    TIM3->CCER |= (1U << 0);        // Enable Capture
}

/* Khởi tạo module siêu âm HC-SR05 */
void HCSR05_Init(void)
{
    /* Enable Clock */
    RCC->AHB1ENR |= (1U << 0);      // GPIOA
    RCC->APB1ENR |= (1U << 1);      // TIM3

    /* Cấu hình chân PA7 (Trig) - Output Push-Pull, High Speed */
    GPIOA->MODER &= ~(3U << (HCSR05_TRIG_PIN * 2U));
    GPIOA->MODER |=  (1U << (HCSR05_TRIG_PIN * 2U));
    GPIOA->OTYPER &= ~(1U << HCSR05_TRIG_PIN);
    GPIOA->OSPEEDR &= ~(3U << (HCSR05_TRIG_PIN * 2U));
    GPIOA->OSPEEDR |=  (2U << (HCSR05_TRIG_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (HCSR05_TRIG_PIN * 2U));
    HCSR05_TrigLow();

    /* Cấu hình chân PA6 (Echo) - Alternate Function (AF2) */
    GPIOA->MODER &= ~(3U << (HCSR05_ECHO_PIN * 2U));
    GPIOA->MODER |=  (2U << (HCSR05_ECHO_PIN * 2U));
    GPIOA->PUPDR &= ~(3U << (HCSR05_ECHO_PIN * 2U));
    GPIOA->AFR[0] &= ~(0xFU << (HCSR05_ECHO_PIN * 4U));
    GPIOA->AFR[0] |=  (HCSR05_TIM3_AF << (HCSR05_ECHO_PIN * 4U));

    /* Cấu hình TIM3 cho Input Capture (1 Tick = 1us) */
    TIM3->CR1 &= ~(1U << 0);        // Dừng Timer
    TIM3->PSC = HCSR05_TIMER_CLK_MHZ - 1U;
    TIM3->ARR = 0xFFFFU;
    TIM3->CNT = 0U;

    /* Cấu hình Channel 1 (TI1) làm Input Capture */
    TIM3->CCMR1 &= ~(3U << 0);
    TIM3->CCMR1 |=  (1U << 0);      // CC1S = 01
    TIM3->CCMR1 &= ~(0xFU << 4);    // Không dùng filter (IC1F = 0000)
    TIM3->CCMR1 &= ~(3U << 2);      // Prescaler = 0 (IC1PSC = 00)

    HCSR05_SetCaptureRisingEdge();
    TIM3->SR &= ~(1U << 1);         // Clear cờ CC1IF
    TIM3->EGR |= (1U << 0);         // Update Event
    TIM3->CR1 |= (1U << 0);         // Start Timer
}

/* Hàm Delay Microsecond dùng TIM3 */
void HCSR05_DelayUs(uint32_t us)
{
    uint32_t start = TIM3->CNT;
    /* Xử lý tràn biến bằng ép kiểu uint16_t */
    while ((uint16_t)(TIM3->CNT - start) < us)
    {
    }
}

/* Đọc độ rộng xung Echo (Raw Data) */
uint32_t HCSR05_ReadEchoPulseUs(void)
{
    uint32_t timeout_start = 0U;
    uint32_t rising_time = 0U;
    uint32_t falling_time = 0U;
    uint32_t pulse_width = 0U;

    TIM3->CNT = 0U;
    HCSR05_SetCaptureRisingEdge();
    TIM3->SR &= ~(1U << 1);

    /* Cấp xung kích Trig (10us) */
    HCSR05_TrigLow();
    HCSR05_DelayUs(2U);
    HCSR05_TrigHigh();
    HCSR05_DelayUs(10U);
    HCSR05_TrigLow();

    /* Đo thời điểm bắt đầu xung Echo (Cạnh lên) */
    timeout_start = TIM3->CNT;
    while (!(TIM3->SR & (1U << 1)))
    {
        if ((uint16_t)(TIM3->CNT - timeout_start) > HCSR05_TIMEOUT_US)
        {
            return 0U; // Timeout Error
        }
    }
    rising_time = TIM3->CCR1;
    TIM3->SR &= ~(1U << 1);

    /* Đo thời điểm kết thúc xung Echo (Cạnh xuống) */
    HCSR05_SetCaptureFallingEdge();
    timeout_start = TIM3->CNT;
    while (!(TIM3->SR & (1U << 1)))
    {
        if ((uint16_t)(TIM3->CNT - timeout_start) > HCSR05_TIMEOUT_US)
        {
            HCSR05_SetCaptureRisingEdge();
            return 0U; // Timeout Error
        }
    }
    falling_time = TIM3->CCR1;
    TIM3->SR &= ~(1U << 1);

    /* Phục hồi cấu hình cho lần đo sau */
    HCSR05_SetCaptureRisingEdge();

    /* Tính toán khoảng thời gian (xử lý tràn Timer) */
    if (falling_time >= rising_time)
    {
        pulse_width = falling_time - rising_time;
    }
    else
    {
        pulse_width = (0xFFFFU - rising_time) + falling_time + 1U;
    }

    if (pulse_width > HCSR05_TIMEOUT_US)
    {
        return 0U;
    }

    return pulse_width;
}

/* Tính toán và trả về khoảng cách vật cản (cm) */
float HCSR05_ReadDistanceCm(void)
{
    uint32_t pulse_us = HCSR05_ReadEchoPulseUs();

    if (pulse_us == 0U)
    {
        return HCSR05_INVALID_DISTANCE;
    }

    /* Vận tốc âm thanh = 340m/s -> Hệ số quy đổi 1cm = 29us (chiều đi và về là 58us) */
    return ((float)pulse_us) / 58.0f;
}
