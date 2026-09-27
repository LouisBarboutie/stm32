#include "stm32l432xx.h"
#include "stm32l4xx_it.h"

namespace led
{

void Init()
{
    // GPIOB Enable
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk;

    GPIOB->MODER &= ~GPIO_MODER_MODE3_Msk;
    GPIOB->MODER |= (0b01 << GPIO_MODER_MODE3_Pos) & GPIO_MODER_MODE3_Msk;
    GPIOB->OSPEEDR |= (0b10 << GPIO_OSPEEDR_OSPEED7_Pos) & GPIO_OSPEEDR_OSPEED7_Msk;
}

void On()
{
    GPIOB->ODR |= GPIO_ODR_OD3_Msk;
}

void Off()
{
    GPIOB->ODR &= ~GPIO_ODR_OD3_Msk;
}

void Toggle()
{
    GPIOB->ODR ^= GPIO_ODR_OD3_Msk;
}

} // namespace led

namespace timer
{

bool isRunning = false;

void Init()
{
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM7EN_Msk; // TIM7 Enable
    NVIC_EnableIRQ(TIM7_IRQn);

    TIM7->DIER |= TIM_DIER_UIE_Msk; // Update interrupt enable
    TIM7->PSC = 63;
    TIM7->ARR = 62'500;
}

void Start()
{
    TIM7->CR1 |= TIM_CR1_CEN_Msk; // Counter enable
    isRunning = true;
}

void Stop()
{
    TIM7->CR1 &= ~TIM_CR1_CEN_Msk; // Counter disable
    isRunning = false;
}

void Reset()
{
    TIM7->CNT = 0;
}

} // namespace timer

namespace clock
{

void Init()
{
    // Target HCLK frequency is 8 MHz, from SYSCLK 16 MHz. Wait states for flash
    // is 0 for <= 16 MHz

    // Turn HSI clock On
    RCC->CR &= ~RCC_CR_HSION_Msk;
    RCC->CR |= RCC_CR_HSION_Msk;

    // Wait until HSI clock is ready
    while ((RCC->CR & RCC_CR_HSIRDY_Msk) == 0)
        ;

    // Set AHB prescaler to /2
    RCC->CFGR &= ~RCC_CFGR_HPRE_Msk;
    RCC->CFGR |= RCC_CFGR_HPRE_DIV2;

    // Turn PLL Off
    RCC->CR &= ~RCC_CR_PLLON_Msk;

    // Wait until PLL is ready
    while ((RCC->CR & RCC_CR_PLLRDY_Msk) != 0)
        ;

    // Select HSI as input source for the PLL
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC_Msk;
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;

    // Set M to /2
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_Msk;
    RCC->PLLCFGR |= (1 << RCC_PLLCFGR_PLLM_Pos) & RCC_PLLCFGR_PLLM_Msk;

    // Set R to /8
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR_Msk;
    RCC->PLLCFGR |= (0b11 << RCC_PLLCFGR_PLLR_Pos) & RCC_PLLCFGR_PLLR_Msk;

    // Set N to x16
    RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_Msk;
    RCC->PLLCFGR |= (0x10 << RCC_PLLCFGR_PLLN_Pos) & RCC_PLLCFGR_PLLN_Msk;

    // Turn PLL On
    RCC->CR |= RCC_CR_PLLON_Msk;

    // PLLR Enable output
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN_Msk;

    // Wait until PLL is ready
    while ((RCC->CR & RCC_CR_PLLRDY_Msk) == 0)
        ;

    // Select PLL as SYSCLK source
    RCC->CFGR &= ~RCC_CFGR_SW_Msk;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
}

} // namespace clock

namespace gpio
{

void Init()
{
    // GPIOB port clock enable
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN_Msk;

    // Set PB7 mode to input
    GPIOB->MODER &= ~GPIO_MODER_MODE7_Msk;

    // Activate PB7 pull-down resistor
    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7_Msk;
    GPIOB->PUPDR |= (0b10 << GPIO_PUPDR_PUPD7_Pos) & GPIO_PUPDR_PUPD7_Msk;

    // Enable line 7 interrupt
    EXTI->IMR1 |= EXTI_IMR1_IM7_Msk;

    // Trigger on rising edge
    EXTI->RTSR1 |= EXTI_RTSR1_RT7_Msk;

    // Enable clock for SYSCFG
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN_Msk;

    // Select PB7 for line 7 interrupt
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI7_Msk;
    SYSCFG->EXTICR[1] |= (0b001 << SYSCFG_EXTICR2_EXTI7_Pos) & SYSCFG_EXTICR2_EXTI7_Msk;

    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

} // namespace gpio

extern "C" void TIM7_IRQHandler()
{
    led::Toggle();
    TIM7->SR &= ~TIM_SR_UIF_Msk; // Update interrupt flag clear
}

extern "C" void EXTI9_5_IRQHandler()
{
    // Discard other interrupts
    if ((EXTI->PR1 & EXTI_PR1_PIF7_Msk) == 0)
        return;

    if (timer::isRunning)
        timer::Stop();
    else
        timer::Start();

    // Clear the pending interrupt
    EXTI->PR1 |= EXTI_PR1_PIF7_Msk;
}

extern "C" SysTick_Handler()
{
}

extern "C" void cpp_main()
{
    clock::Init();
    led::Init();
    timer::Init();
    gpio::Init();

    while (1)
        ;
}