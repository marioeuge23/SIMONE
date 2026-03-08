/**
 * @file interr.c
 * @brief Interrupt service routines for the STM32F4 platform.
 * @author SDG2. Román Cárdenas (r.cardenas@upm.es) and Josué Pagán (j.pagan@upm.es)
 * @date 2026-01-01
 */
// Include HW dependencies:
#include "port_system.h"
#include "stm32f4_system.h"
#include "stm32f4_button.h"
#include "port_button.h"
#include "stm32f4xx.h"

// Include headers of different port elements:

//------------------------------------------------------
// INTERRUPT SERVICE ROUTINES
//------------------------------------------------------
/**
 * @brief Interrupt service routine for the System tick timer (SysTick).
 *
 * @note This ISR is called when the SysTick timer generates an interrupt.
 * The program flow jumps to this ISR and increments the tick counter by one millisecond.
 *
 * > **TO-DO alumnos:**
 * >
 * > ✅ 1. **Increment the System tick counter `msTicks` in 1 count.** To do so, use the function `port_system_get_millis()` and `port_system_set_millis()`.
 *
 * @warning **The variable `msTicks` must be declared volatile!** Just because it is modified by a call of an ISR, in order to avoid [*race conditions*](https://en.wikipedia.org/wiki/Race_condition). **Added to the definition** after *static*.
 *
 */
void SysTick_Handler(void)
{
    uint32_t ms = port_system_get_millis();
    port_system_set_millis(ms + 1);
}

/**
 * @brief Interrupt service routine for the EXTI line 15 to 10.
 * 
 * 
 */
void EXTI15_10_IRQHandler(void)
{
    /* ISR user button */
    if (EXTI->PR & BIT_POS_TO_MASK(buttons_arr[PORT_USER_BUTTON_ID].pin))
    {
        GPIO_TypeDef *p_port = buttons_arr[PORT_USER_BUTTON_ID].p_port;
        uint8_t pin = buttons_arr[PORT_USER_BUTTON_ID].pin;
        bool activated = stm32f4_system_gpio_read(p_port, pin);
        //Activado por flanco de bajada, el pin se pone a 0 cuando se pulsa el botón, por lo que si el pin está activado, el botón no está pulsado
        if(activated)
        {
            buttons_arr[PORT_USER_BUTTON_ID].flag_pressed = false;
        }
        else
        {
            buttons_arr[PORT_USER_BUTTON_ID].flag_pressed = true;
        }
        //Clear pending bit of the EXTI line associated with the USER BUTTON pin
        EXTI->PR = BIT_POS_TO_MASK(buttons_arr[PORT_USER_BUTTON_ID].pin); 
    }
}