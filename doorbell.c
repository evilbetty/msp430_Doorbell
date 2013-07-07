#include "msp430.h"
#include <stdint.h>
#include "binary_data.h"        // Include our audio data

#define MCLK                    16000000
#define SAMPLES_PER_SECOND      8000
#define BUTTON                  BIT3
#define PIN_SPEAKER             BIT2

// Get the size of our data array
const uint16_t binary_data_size = sizeof( binary_data ) / sizeof( binary_data[0] );

// Data location (start higher than binary_data_size to not play at reset)
volatile uint16_t counter = ( sizeof( binary_data ) / sizeof( binary_data[0] ) ) + 1;

// Sample buffer
volatile uint8_t sample;


int main(void) {
    // Disable WDT and run with 8Mhz MCLK
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = CALDCO_16MHZ;
    BCSCTL1 = CALBC1_16MHZ;

    // PIN2 as timer output
    P1SEL |= PIN_SPEAKER;
    P1DIR |= PIN_SPEAKER;

    // Button as input, output high for pullup, and pullup on
    P1DIR &= ~BUTTON;
    P1OUT |= BUTTON;
    P1REN |= BUTTON;
    // P1 interrupt enabled with P1.3 as trigger (raising edge)
    P1IE |= BUTTON;
    P1IES |= BUTTON;
    // P1.3 interrupt flag cleared.
    P1IFG &= ~BUTTON;

    //  Timer 0 for output PWM (for 8bit values)
    TA0CTL = TASSEL_2 | MC_1;
    TA0CCR0 = 255;
    TA0CCTL1 |= OUTMOD_7;
    TA0CCR1 = 0;    // Added for 0 output after reset

    // Timer 1 to interrupt at sample speed
    TA1CTL = TASSEL_2 | MC_1;
    TA1CCR0 = MCLK / SAMPLES_PER_SECOND - 1;
    TA1CCTL0 |= CCIE;

    _BIS_SR(GIE);       // Enable global interrupts

	while(1){
        
        if ( counter <= binary_data_size ) 
        {   // If data is still playing do LPM0 to keep
            // the timer running and audio playing.
            sample = binary_data[counter++];
            LPM0;
        }
        else 
        {
            // If done playing go to LPM4 so that only
            // the button wakes MCU up.
            sample = 0;
            TA0CCR1 = 0;         // Added for guaranteed 0 output.
            __delay_cycles(500); // Needed to not have random pin output
            // We really dont want Vcc going through speaker when idle
            LPM4;
        }
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void T1A0_ISR(void)
{
    // Output sample and wake mcu to get next sample ready
    TA0CCR1 = sample;
    LPM0_EXIT;
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    // Set playnow var when button pressed (and a start value)
    counter = 1024;     // Our audio wastes a whole KB of begin silence
    P1IFG &= ~BUTTON;   // Clearing IFG. This is required.
    LPM4_EXIT;
}
