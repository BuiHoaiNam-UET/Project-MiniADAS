#include "led.h"

/* Khởi tạo GPIO cho LED (PC0) */
void LED_Init(void)
{
    /* Enable Clock cho GPIOC */
    RCC->AHB1ENR |= (1 << 2);

    /* Cấu hình PC0 Mode: Output (01) */
    LED_GPIO_PORT->MODER &= ~(3 << (LED_PIN * 2));
    LED_GPIO_PORT->MODER |=  (1 << (LED_PIN * 2));

    /* Output Type: Push-pull */
    LED_GPIO_PORT->OTYPER &= ~(1 << LED_PIN);

    /* Output Speed: Low */
    LED_GPIO_PORT->OSPEEDR &= ~(3 << (LED_PIN * 2));

    /* No Pull-up/Pull-down */
    LED_GPIO_PORT->PUPDR &= ~(3 << (LED_PIN * 2));

    /* Trạng thái mặc định: Tắt */
    LED_Off();
}

/* Bật LED (Active-High) */
void LED_On(void)
{
    LED_GPIO_PORT->ODR |= (1 << LED_PIN);
}

/* Tắt LED */
void LED_Off(void)
{
    LED_GPIO_PORT->ODR &= ~(1 << LED_PIN);
}

/* Đảo trạng thái (Toggle) */
void LED_Toggle(void)
{
    LED_GPIO_PORT->ODR ^= (1 << LED_PIN);
}

/* Điều khiển LED cảnh báo theo ngưỡng khoảng cách */
void LED_UpdateByDistance(float distance_cm, float threshold_cm)
{
    /* Bật LED cảnh báo nếu lỗi đo (distance < 0) hoặc nằm trong ngưỡng nguy hiểm */
    if ((distance_cm < 0.0f) || (distance_cm < threshold_cm))
    {
        LED_On();
    }
    else
    {
        LED_Off();
    }
}
