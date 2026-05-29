#include "hcsr05.h"

/* Cấu hình chân phần cứng */
#define HCSR05_TRIG_PIN 7  // PA7: GPIO Output
#define HCSR05_ECHO_PIN 6  // PA6: TIM3_CH1 (Input Capture)
#define HCSR05_TIM3_AF 2  // Alternate Function 2 cho TIM3

/* Các hàm thao tác mức vật lý cho chân Trig */
static void HCSR05_TrigHigh(void)
{
    GPIOA->ODR |= (1 << HCSR05_TRIG_PIN);
}

static void HCSR05_TrigLow(void)
{
    GPIOA->ODR &= ~(1 << HCSR05_TRIG_PIN);
}

/* Cấu hình Input Capture bắt cạnh lên (Start of Echo) */
static void HCSR05_SetCaptureRisingEdge(void)
{
    TIM3->CCER &= ~(1 << 0);       // Disable Capture
    TIM3->CCER &= ~(1 << 1);       // CC1P = 0
    TIM3->CCER &= ~(1 << 3);       // CC1NP = 0 (Rising edge)
    TIM3->CCER |= (1 << 0);        // Enable Capture
}

/* Cấu hình Input Capture bắt cạnh xuống (End of Echo) */
static void HCSR05_SetCaptureFallingEdge(void)
{
    TIM3->CCER &= ~(1 << 0);       // Disable Capture
    TIM3->CCER |= (1 << 1);        // CC1P = 1
    TIM3->CCER &= ~(1 << 3);       // CC1NP = 0 (Falling edge)
    TIM3->CCER |= (1 << 0);        // Enable Capture
}

/* Khởi tạo module siêu âm HC-SR05 */
void HCSR05_Init(void)
{
    /* Enable Clock */
    RCC->AHB1ENR |= (1 << 0);      // GPIOA
    RCC->APB1ENR |= (1 << 1);      // TIM3

    /* Cấu hình chân PA7 (Trig) - Output Push-Pull, High Speed */
    GPIOA->MODER &= ~(3 << (HCSR05_TRIG_PIN * 2));
    GPIOA->MODER |=  (1 << (HCSR05_TRIG_PIN * 2));
    GPIOA->OTYPER &= ~(1 << HCSR05_TRIG_PIN);
    GPIOA->OSPEEDR &= ~(3 << (HCSR05_TRIG_PIN * 2));
    GPIOA->OSPEEDR |=  (2 << (HCSR05_TRIG_PIN * 2));
    GPIOA->PUPDR &= ~(3 << (HCSR05_TRIG_PIN * 2));
    HCSR05_TrigLow();

    /* Cấu hình chân PA6 (Echo) - Alternate Function (AF2) */
    GPIOA->MODER &= ~(3 << (HCSR05_ECHO_PIN * 2));
    GPIOA->MODER |=  (2 << (HCSR05_ECHO_PIN * 2));
    GPIOA->PUPDR &= ~(3 << (HCSR05_ECHO_PIN * 2));
    GPIOA->AFR[0] &= ~(0xF << (HCSR05_ECHO_PIN * 4));
    GPIOA->AFR[0] |=  (HCSR05_TIM3_AF << (HCSR05_ECHO_PIN * 4));

    /* Cấu hình TIM3 cho Input Capture (1 Tick = 1us) */
    TIM3->CR1 &= ~(1 << 0);        // Dừng Timer
    TIM3->PSC = HCSR05_TIMER_CLK_MHZ - 1;
    TIM3->ARR = 0xFFFF;
    TIM3->CNT = 0;

    /* Cấu hình Channel 1 (TI1) làm Input Capture */
    TIM3->CCMR1 &= ~(3 << 0);
    TIM3->CCMR1 |=  (1 << 0);      // CC1S = 01
    TIM3->CCMR1 &= ~(0xF << 4);    // Không dùng filter (IC1F = 0000)
    TIM3->CCMR1 &= ~(3 << 2);      // Prescaler = 0 (IC1PSC = 00)

    HCSR05_SetCaptureRisingEdge();
    TIM3->SR &= ~(1 << 1);         // Clear cờ CC1IF
    TIM3->EGR |= (1 << 0);         // Update Event
    TIM3->CR1 |= (1 << 0);         // Start Timer
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
    uint32_t timeout_start = 0;
    uint32_t rising_time = 0;
    uint32_t falling_time = 0;
    uint32_t pulse_width = 0;

    TIM3->CNT = 0;
    HCSR05_SetCaptureRisingEdge();
    TIM3->SR &= ~(1 << 1);

    /* Cấp xung kích Trig (10us) */
    HCSR05_TrigLow();
    HCSR05_DelayUs(2);
    HCSR05_TrigHigh();
    HCSR05_DelayUs(10);
    HCSR05_TrigLow();

    /* Đo thời điểm bắt đầu xung Echo (Cạnh lên) */
    timeout_start = TIM3->CNT;
    while (!(TIM3->SR & (1 << 1)))
    {
        if ((uint16_t)(TIM3->CNT - timeout_start) > HCSR05_TIMEOUT_US)
        {
            return 0; // Timeout Error
        }
    }
    rising_time = TIM3->CCR1;
    TIM3->SR &= ~(1 << 1);

    /* Đo thời điểm kết thúc xung Echo (Cạnh xuống) */
    HCSR05_SetCaptureFallingEdge();
    timeout_start = TIM3->CNT;
    while (!(TIM3->SR & (1 << 1)))
    {
        if ((uint16_t)(TIM3->CNT - timeout_start) > HCSR05_TIMEOUT_US)
        {
            HCSR05_SetCaptureRisingEdge();
            return 0; // Timeout Error
        }
    }
    falling_time = TIM3->CCR1;
    TIM3->SR &= ~(1 << 1);

    /* Phục hồi cấu hình cho lần đo sau */
    HCSR05_SetCaptureRisingEdge();

    /* Tính toán khoảng thời gian (xử lý tràn Timer) */
    if (falling_time >= rising_time)
    {
        pulse_width = falling_time - rising_time;
    }
    else
    {
        pulse_width = (0xFFFF - rising_time) + falling_time + 1;
    }

    if (pulse_width > HCSR05_TIMEOUT_US)
    {
        return 0;
    }

    return pulse_width;
}

/* Tính toán và trả về khoảng cách vật cản (cm) */
float HCSR05_ReadDistanceCm(void)
{
    uint32_t pulse_us = HCSR05_ReadEchoPulseUs();

    if (pulse_us == 0)
    {
        return HCSR05_INVALID_DISTANCE;
    }

    /* Vận tốc âm thanh = 340m/s -> Hệ số quy đổi 1cm = 29us (chiều đi và về là 58us) */
    return ((float)pulse_us) / 58.0f;
}
