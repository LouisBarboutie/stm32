#include "stm32l432xx.h"

namespace led
{

void Init()
{
  // GPIOB Enable
  RCC->AHB2ENR |= 0x00000002;

  GPIOB->MODER &= ~0x0000'00C0;
  GPIOB->MODER |= 0x0000'0040;
  GPIOB->OSPEEDR |= 0x0000'0040;
}

void On()
{
  GPIOB->ODR |= 0X8;
}

void Off()
{
  GPIOB->ODR &= ~0x8;
}

void Toggle()
{
  GPIOB->ODR ^= (1 << 3);
}

} // namespace led

namespace timer
{

void Init()
{
  RCC->APB1ENR1 |= (1 << 5); // TIM7 Enable
  NVIC_EnableIRQ(TIM7_IRQn);

  TIM7->CR1 |= 0b1;  // Counter enable
  TIM7->DIER |= 0b1; // Update interrupt enable
  TIM7->PSC = 63;
  TIM7->ARR = 62'500;
}

} // namespace timer

extern "C" void TIM7_IRQHandler()
{
  led::Toggle();
  TIM7->SR &= ~0b1; // Update interrupt flag clear
}

extern "C" void cpp_main()
{
  led::Init();
  timer::Init();

  while (1)
    ;
}