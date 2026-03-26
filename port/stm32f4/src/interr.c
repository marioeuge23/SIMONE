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
#include "port_keyboard.h"
#include "stm32f4_keyboard.h"

//------------------------------------------------------
// PRIVATE FUNCTIONS :)
//------------------------------------------------------

/**
 * @brief Comprueba y maneja la interrupcion de una columna
 * 
 * @param column_index
 */
static void _check_column_interrupt(uint8_t column_index)
{
    /* 1. Get the GPIO port and pin */
    GPIO_TypeDef *p_port = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_ports[column_index];
    uint8_t pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[column_index];

    /* 2. Read the GPIO pin state */
    bool is_high = stm32f4_system_gpio_read(p_port, pin);

    /* 3. Update flag_key_pressed and col_idx_interrupt */
    if (is_high) {
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].flag_key_pressed = true;
    } else {
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].flag_key_pressed = false;
        keyboards_arr[PORT_KEYBOARD_MAIN_ID].col_idx_interrupt = column_index;
    }

    /* 4. Clear the interrupt pending flag */
    EXTI->PR = BIT_POS_TO_MASK(pin);
}

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

    uint8_t col1_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_1];
    if (EXTI->PR & BIT_POS_TO_MASK(col1_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_1);
    }
}

/**
 * @brief This function handles EXTI lines [9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void)
{
    /* Keyboard Column 0 and 3 */
    
    /* Check Column 0 */
    uint8_t col0_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_0];
    if (EXTI->PR & BIT_POS_TO_MASK(col0_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_0);
    }

    /* Check Column 3 */
    uint8_t col3_pin = keyboards_arr[PORT_KEYBOARD_MAIN_ID].p_col_pins[PORT_KEYBOARD_COL_3];
    if (EXTI->PR & BIT_POS_TO_MASK(col3_pin))
    {
        _check_column_interrupt(PORT_KEYBOARD_COL_3);
    }
}

/**
 * @brief This function handles EXTI line 4 interrupt.
 */
void EXTI4_IRQHandler(void)
{
    /* Keyboard Column 2 */
    /* Only one line triggers this ISR, so no need for 'if' */
    _check_column_interrupt(PORT_KEYBOARD_COL_2);
}

/**
 * @brief Interrupt service routine for the TIM5 timer.
 */
void TIM5_IRQHandler(void)
{
    /* Row scanning timeout */
    if (TIM5->SR & TIM_SR_UIF) {
        /* 1. Clear the interrupt flag UIF */
        TIM5->SR &= ~TIM_SR_UIF;
        
        /* 2. Set the row timeout flag */
        port_keyboard_set_row_timeout_status(PORT_KEYBOARD_MAIN_ID, true);
    }
}