/*---------------------------------------------------------------------------
 * Copyright (c) 2025-2026 Arm Limited (or its affiliates).
 * All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    retarget_stdio.c
 *      Purpose: Retarget stdio to CMSIS UART
 *
 *---------------------------------------------------------------------------*/

#include "RTE_Components.h"
#include CMSIS_target_header
#include "cmsis_os2.h"

/* Compile-time configuration */
#ifndef UART_BAUDRATE
#define UART_BAUDRATE 115200
#endif

/* Exported functions */
extern int stdio_init(void);
extern int stdin_getchar(void);
extern int stdout_putchar(int ch);
extern int stderr_putchar(int ch);

/* Reference to the underlying USART driver */
#define ptrUSART (&ARM_Driver_USART_(RETARGET_STDIO_UART))

/**
  Initialize stdio

  \return          0 on success, or -1 on error.
*/
int stdio_init(void)
{

    if (ptrUSART->Initialize(NULL) != ARM_DRIVER_OK) {
        return -1;
    }

    if (ptrUSART->PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK) {
        return -1;
    }

    if (ptrUSART->Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 |
                          ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1 |
                          ARM_USART_FLOW_CONTROL_NONE,
                          UART_BAUDRATE) != ARM_DRIVER_OK) {
        return -1;
    }

#if defined(RTE_CMSIS_Compiler_STDIN_Custom)
    if (ptrUSART->Control(ARM_USART_CONTROL_RX, 1U) != ARM_DRIVER_OK) {
        return -1;
    }
#endif

#if defined(RTE_CMSIS_Compiler_STDERR_Custom) || defined(RTE_CMSIS_Compiler_STDOUT_Custom)
    if (ptrUSART->Control(ARM_USART_CONTROL_TX, 1U) != ARM_DRIVER_OK) {
        return -1;
    }
#endif

    return 0;
}

#if defined(RTE_CMSIS_Compiler_STDIN_Custom)
/**
  Get a character from stdin

  \return     The next character from the input, or -1 on read error.
*/
int stdin_getchar(void)
{
    uint8_t buf[1];

    if (ptrUSART->Receive(buf, 1) != ARM_DRIVER_OK) {
        return (-1);
    }
    while (ptrUSART->GetRxCount() != 1) {
        osDelay(10U);
    }
    return (buf[0]);
}
#endif

#if defined(RTE_CMSIS_Compiler_STDOUT_Custom)
/**
  Put a character to the stdout

  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stdout_putchar(int ch)
{
    uint8_t buf[1];

    buf[0] = (uint8_t) ch;
    if (ptrUSART->Send(buf, 1) != ARM_DRIVER_OK) {
        return (-1);
    }
    while (ptrUSART->GetTxCount() != 1) {
    }
    return ch;
}
#endif

#if defined(RTE_CMSIS_Compiler_STDERR_Custom)
/**
  Put a character to the stderr

  \param[in]   ch  Character to output
  \return          The character written, or -1 on write error.
*/
int stderr_putchar(int ch)
{
    uint8_t buf[1];

    buf[0] = (uint8_t) ch;
    if (ptrUSART->Send(buf, 1) != ARM_DRIVER_OK) {
        return (-1);
    }
    while (ptrUSART->GetTxCount() != 1) {
    }
    return ch;
}
#endif
