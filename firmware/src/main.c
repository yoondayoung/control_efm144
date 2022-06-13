/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>

#define DEBUG 1                         // 1: debug mode(enable functional print), 0: release mode
// spi data define
#define CONFIG4 0
#define RLY_NEG_ON 1
#if DEBUG==1
#define MPRLY_ON 2
#define MNRLY_ON 3
#else
#define MPRLY_ON 4
#define MNRLY_ON 5
#endif


// global variables
volatile uint32_t global_tick = 0;      // global tick for control, 1 tick = 10ms
volatile int state = 0;                 // state for control which stores power state of Relay/IGBT
void (*turnon_sequence[4])();           // turn on sequence function pointer list
void (*turnoff_sequence[2])();          // turn off sequence function pointer list
// for spi data
int txData[5]  = { 0b10000000000000000000000000011001,
                   0b10001101000000000000000110000011, // out 5,6
                   0b10001101000000000000100000000010, // src 1 (M+ Rly Pos)
                   0b10001101000000000001000000000100, // src 2 (M- Rly Pos)
                   0b10001101000000000000100110000100, // src 1 & out 5,6
                   0b10001101000000000001000110000010, // src 2 & out 5,6
                   0b10001101000000000000000000011011}; // out 1,2
int rxData;

/* timer2 interrupt handler */
void TIMER2_InterruptSvcRoutine(uint32_t status, uintptr_t context)
{   
    global_tick++;
}

void mn_igbt_relay_on(){
    MN_IGBT_Set(); 
    state = 1; // state 1 : m- igbt on
    
#if DEBUG==1
    SPI1_WriteRead(&txData[3], 4, &rxData, 4);
    printf("Rx %x\r\n", rxData);
#else
    SPI1_WriteRead(&txData[3], 4, &rxData, 4);
#endif
    state = 2; // state 2 : m- igbt, m- relay on
}

void pc_on(){
    PC_IGBT_Set();
    state = 3; // state 3 : m- igbt, m- relay, pc igbt on
}

void mp_igbt_relay_on(){
    MP_IGBT_Set();
    state = 4; // state 4 : m- igbt, m- relay, pc igbt, m+ igbt on
    SPI1_WriteRead(&txData[2], 4, &rxData, 4);
#if DEBUG==1
    printf("Rx %x\r\n", rxData);
#endif
    state = 5; // state 5 : m- igbt, m- relay, pc igbt, m+ igbt, m+ relay on
}

void pc_off(){
    PC_IGBT_Clear();
    state = 6; // state 6 : m- igbt, m- relay, m+ igbt, m+ relay on
}

void mn_mp_relay_off(){
    MN_Relay_Clear();
//    GPIO_RB12_Clear();
    MP_Relay_Clear();
//    GPIO_RB13_Clear();
    state = 7; // state 7 : m- igbt, m- relay, m+ igbt, m+ relay on
}
void mn_mp_igbt_off(){
    MN_IGBT_Clear();
//    GPIO_RH0_Clear(); 
    MP_IGBT_Clear();
//    GPIO_RH2_Clear();
    state = 0; // state 0
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* SPI Write : relay board configuration */
    SPI1_WriteRead(&txData[CONFIG4], 4, &rxData, 4);
#if DEBUG==1
    printf("%x\r\n", rxData);
#endif
    /* SPI Write : power on low side(out 5,6) */
    SPI1_WriteRead(&txData[RLY_NEG_ON], 4, &rxData, 4);
#if DEBUG==1
    printf("%x\r\n", rxData);
#endif

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

