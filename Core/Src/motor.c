#include "motor.h"

// Gọi TIM2 để băm xung
extern TIM_HandleTypeDef htim2;

void Motor_Init(void) {
    // 1. Cấp điện cho cả 3 cổng Port A, B, C (Vị chân điều hướng tản mạn ở cả 3 port)
    RCC->AHB1ENR |= (1U << 0) | (1U << 1) | (1U << 2);

    // 2. Ép PA8, PA9, PC7, PB6 thành GPIO Output (mã 01)
    // IN1: PA8
    GPIOA->MODER &= ~(3U << (8 * 2));
    GPIOA->MODER |= (1U << (8 * 2));

    // IN2: PA9
    GPIOA->MODER &= ~(3U << (9 * 2));
    GPIOA->MODER |= (1U << (9 * 2));

    // IN3: PC7
    GPIOC->MODER &= ~(3U << (7 * 2));
    GPIOC->MODER |= (1U << (7 * 2));

    // IN4: PB6
    GPIOB->MODER &= ~(3U << (6 * 2));
    GPIOB->MODER |= (1U << (6 * 2));

    // 3. Đánh thức TIM2 băm xung cho ENA (PA0) và ENB (PA1)
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

    // 4. Khóa cứng bánh lúc vừa cắm điện
    Motor_Stop();
}

void Motor_Forward(uint8_t speedLeft, uint8_t speedRight) {
    // Bánh trái tiến: IN1(PA8)=1, IN2(PA9)=0
    GPIOA->ODR |= (1U << 8);
    GPIOA->ODR &= ~(1U << 9);

    // Bánh phải tiến: IN3(PC7)=1, IN4(PB6)=0
    GPIOC->ODR |= (1U << 7);
    GPIOB->ODR &= ~(1U << 6);

    // Bơm tốc độ vào TIM2
    TIM2->CCR1 = speedLeft;  // PA0
    TIM2->CCR2 = speedRight; // PA1
}

void Motor_Stop(void) {
    // Dập điện cả 4 chân điều hướng
    GPIOA->ODR &= ~((1U << 8) | (1U << 9));
    GPIOC->ODR &= ~(1U << 7);
    GPIOB->ODR &= ~(1U << 6);

    // Rút PWM về 0
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
}
