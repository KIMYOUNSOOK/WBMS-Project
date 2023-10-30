/*******************************************************************************
* @brief    Print utilities
*
* @details  Set of printf-like functions which send their output to host device's UART-to_USB bridge
*
* Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
* This software is proprietary and confidential to Analog Devices, Inc. and its licensors.
*******************************************************************************/
#include "adi_wil_example_printf.h"
#include <stdarg.h>
#include "IfxAsclin_Asc.h"
#include "adi_wil_example_isr_priorities.h"


#define ASCLIN_TX_BUFFER_SIZE (256)         /* Number of characters in the transmit buffer */
#define ASCLIN_RX_BUFFER_SIZE (256)         /* Number of characters in the receive buffer */
#define ASCLIN_RX_BUFFER_PADDING (8)        /* Buffers require this much internal padding to guarantee 64-bit boundary. */

#if !defined(ASCLIN_BAUDRATE)
#define ASCLIN_BAUDRATE    (921600)      /* UART baud rate (can be overridden by application) */
#endif

#if !defined(ASCLIN_ENDLINE)
#define ASCLIN_ENDLINE        "\r\n"        /* Macro for end-of-line character sequence (can be overridden by application) */
#endif

/* Application Asclin instance */
static IfxAsclin_Asc asc;

/*
    Transmit buffer. Per documentation, sizeof(Ifx_Fifo) bytes are required by platform for buffer
    management, plus additional padding to guarantee necessary word boundaries.
*/
static uint8 ascTxBuffer[ASCLIN_TX_BUFFER_SIZE + sizeof(Ifx_Fifo) + ASCLIN_RX_BUFFER_PADDING];
/*
    Receive buffer. Per documentation, sizeof(Ifx_Fifo) bytes are required by platform for buffer
    management, plus additional padding to guarantee necessary word boundaries.
*/
static uint8 ascRxBuffer[ASCLIN_RX_BUFFER_SIZE + sizeof(Ifx_Fifo) + ASCLIN_RX_BUFFER_PADDING];



/* Tx ISR. When called, it refills the ASCLIN hardware FIFO (16 bytes) with characters from the transmit buffer */
IFX_INTERRUPT(asclin0TxISR, 0, ISR_PRIORITY_ASCLIN3_TX)
{
    IfxAsclin_Asc_isrTransmit(&asc);
}

/* Minimal Rx ISR, stores chars received to the receive buffer */
IFX_INTERRUPT(asclin0RxISR, 0, ISR_PRIORITY_ASCLIN3_RX)
{
    IfxAsclin_Asc_isrReceive(&asc);
}

/* Minimal Error ISR, error flags are logged in the given IfxAsclin_Asc instance and can be inspected if needed. */
IFX_INTERRUPT(asclin0ErISR, 0, ISR_PRIORITY_ASCLIN3_ER)
{
    IfxAsclin_Asc_isrError(&asc);
}


/* Pin Configuration. UART-to-USB bridge connects to pins P32.2 and P15.7 */
const IfxAsclin_Asc_Pins PrintfPins =
{
        .cts        = NULL_PTR,                         /* CTS pin not used                                */
        .ctsMode    = IfxPort_InputMode_pullUp,
        .rx         = &IfxAsclin5_RXC_P22_3_IN ,        /* Select the pin for RX connected to the USB port */
        .rxMode     = IfxPort_InputMode_pullUp,         /* RX pin                                          */
        .rts        = NULL_PTR,                         /* RTS pin not used                                */
        .rtsMode    = IfxPort_OutputMode_pushPull,
        .tx         = &IfxAsclin5_TX_P22_2_OUT,         /* Select the pin for TX connected to the USB port */
        .txMode     = IfxPort_OutputMode_pushPull,      /* TX pin                                          */
        .pinDriver  = IfxPort_PadDriver_cmosAutomotiveSpeed1
};


/*
 * Configures ASCLIN module 3 to transmit & receive in UART mode at the application baud rate.
 * Module 3 is used b/c the UART-to-USB bridge connects to this module
 *
 * @arguments : none
 *
 * @return none
 */
void adi_wil_ex_printfInit(void)
{
    /* create module config and populate with defaults. The defaults give us UART mode in 8-1-none format. */
    IfxAsclin_Asc_Config ascConfig;
    IfxAsclin_Asc_initModuleConfig(&ascConfig, &MODULE_ASCLIN5);

    /* Set baudrate */
    ascConfig.baudrate.baudrate = ASCLIN_BAUDRATE;                              /* Set the baud rate in bit/s  */
    ascConfig.baudrate.oversampling = IfxAsclin_OversamplingFactor_16;          /* Set the oversampling factor */

    /* Configure the sampling mode (only affects Rx but the defaults are a bit weak) */
    ascConfig.bitTiming.medianFilter = IfxAsclin_SamplesPerBit_three;           /* Set the number of samples per bit */
    ascConfig.bitTiming.samplePointPosition = IfxAsclin_SamplePointPosition_8;  /* Set the first sample position     */

    /* Set ISR priorities and interrupt target */
    ascConfig.interrupt.txPriority = ISR_PRIORITY_ASCLIN3_TX;
    ascConfig.interrupt.rxPriority = ISR_PRIORITY_ASCLIN3_RX;
    ascConfig.interrupt.erPriority = ISR_PRIORITY_ASCLIN3_ER;
    ascConfig.interrupt.typeOfService = IfxSrc_Tos_cpu0;             /* interrupts will be serviced on CPU 0 */

    /* Configure FIFOs */
    ascConfig.txBuffer = &ascTxBuffer;
    ascConfig.txBufferSize = ASCLIN_TX_BUFFER_SIZE;
    ascConfig.rxBuffer = &ascRxBuffer;
    ascConfig.rxBufferSize = ASCLIN_RX_BUFFER_SIZE;

    /* Pass in pin configurations */
    ascConfig.pins = &PrintfPins;

    /* Initialize module  */
    IfxAsclin_Asc_initModule(&asc, &ascConfig);
}

/*
 * Application printf().
 * __format__ tells the compiler to treat function arguments same as printf.
 * Compiler will report mismatches between format string and following arguments.
 *
 * @arguments : same as standard printf()
 *
 * @return number of characters printed
 */
int adi_wil_ex_printf(const char * format, ...) __format__(printf,1,2)
{
    va_list args;
    va_start(args, format);
    int iCharCount = vprintf(format, args);
    va_end(args);

    return iCharCount;
}

/*
 * A variation of adi_wil_ex_printf() which reports user information.
 * When called, starts a new line with information prefix, followed by application message.
 *
 * @arguments : same as standard printf()
 *
 * @return number of characters printed
 */
int adi_wil_ex_info(const char * format, ...) __format__(printf,1,2)
{
    int iCharCount = printf(ASCLIN_ENDLINE "[INFO] ");
    va_list args;
    va_start(args, format);
    iCharCount += vprintf(format, args);
    va_end(args);

    return iCharCount;
}

/*
 * A variation of adi_wil_ex_printf() which reports an error.
 * When called, starts a new line with error prefix, followed by application message.
 *
 * @arguments : same as standard printf()
 *
 * @return number of characters printed
 */
int adi_wil_ex_error(const char * format, ...) __format__(printf,1,2)
{
    int iCharCount = printf(ASCLIN_ENDLINE "[ERROR] ");
    va_list args;
    va_start(args, format);
    iCharCount += vprintf(format, args);
    va_end(args);

    return iCharCount;
}

/*
 * A variation of adi_wil_ex_printf() which reports a fatal error.
 * When called, starts a new line with fatal prefix, followed by application message.
 *
 * @arguments : same as standard printf()
 *
 * @return number of characters printed
 */
int adi_wil_ex_fatal(const char * format, ...) __format__(printf,1,2)
{
    int iCharCount = printf(ASCLIN_ENDLINE "[FATAL] ");
    va_list args;
    va_start(args, format);
    iCharCount += vprintf(format, args);
    va_end(args);

    return iCharCount;
}

/*
 * Redirects the given character to UART Tx buffer instead of default output stream.
 * Compiler allows applications to override standard IO functions and if overridden,
 * will call application's version instead of the library version. Since standard printf()
 * calls fputc() to output characters, overriding fputc() means all printf() calls are
 * redirected to UART.
 *
 * @arguments : same as standard fputc()
 *
 * @return the character input.
 */
int fputc(int c, FILE * stream)
{
    /* stream parameter will be ignored in this application */
    (void)stream;

    /*
        Writes one character to UART Tx buffer. The name is a little misleading, as
        this only blocks when FIFO is full. It returns immediately if FIFO has space.
    */
    IfxAsclin_Asc_blockingWrite(&asc, (uint8)c);

    return c;
}


bool adi_debug_hal_Getch(char * const gotten)
{
    // Ifx_SizeT readCount = 1;
    // return IfxStdIf_DPipe_read(&g_ascStandardInterface, gotten, &readCount, TIME_INFINITE);  /* block until a char is input */
    // return IfxStdIf_DPipe_read(&g_ascStandardInterface, got, &readCount, TIME_NULL);  // i take time_null means don't wait? i want to wait actually no i don't
    IfxAsclin_Asc_blockingRead(&asc);
    return true;
}

bool adi_debug_hal_IsGetch(char * const gotten)
{
    Ifx_SizeT readCount = 1;
    // return IfxStdIf_DPipe_read(&g_ascStandardInterface, gotten, &readCount, TIME_NULL);  /* don't wait. if a char is waiting, fine. */
    return IfxAsclin_Asc_read(&asc, gotten, &readCount, TIME_NULL);

    // return IfxAsclin_Asc_getReadCount(&asc) != 0;
}
